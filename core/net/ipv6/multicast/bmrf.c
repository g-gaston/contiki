/*
 * Copyright (c) 2014, VUB - ETRO
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         This file implements 'Bidirectional Multicast RPL Forwarding' (BMRF)
 *
 *         It will only work in RPL networks in MOP 3 "Storing with Multicast"
 *
 * \author
 *         Guillermo Gastón
 */

#include "contiki.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ipv6/multicast/uip-mcast6-route.h"
#include "net/ipv6/multicast/uip-mcast6-stats.h"
#include "net/ipv6/multicast/bmrf.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "lib/list.h"
#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#if UIP_CONF_IPV6
/*---------------------------------------------------------------------------*/
/* Macros */
/*---------------------------------------------------------------------------*/
/* CCI */
#define BMRF_FWD_DELAY()  NETSTACK_RDC.channel_check_interval()
/* Number of slots in the next 500ms */
#define BMRF_INTERVAL_COUNT  ((CLOCK_SECOND >> 2) / fwd_delay)
/*---------------------------------------------------------------------------*/
/* Internal Data */
/*---------------------------------------------------------------------------*/
static struct ctimer mcast_periodic;
static uint8_t mcast_len;
static uip_buf_t mcast_buf;
static uint8_t fwd_delay;
static uint8_t fwd_spread;
/*---------------------------------------------------------------------------*/
/* uIPv6 Pointers */
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_HBHO_BUF              ((struct uip_hbho_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_EXT_HDR_OPT_RPL_BUF   ((struct uip_ext_hdr_opt_rpl *)&uip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
/*---------------------------------------------------------------------------*/
static void
mcast_fwd(void *p)
{
  memcpy(uip_buf, &mcast_buf, mcast_len);
  uip_len = mcast_len;
  UIP_IP_BUF->ttl--;
  tcpip_output(NULL);
  uip_len = 0;
}
static void
mcast_fwd_with_broadcast(void *p)
{
  /* If we enter here, we will definitely forward */
  UIP_MCAST6_STATS_ADD(mcast_fwd);

  /*
   * Add a delay (D) of at least BMRF_FWD_DELAY() to compensate for how
   * contikimac handles broadcasts. We can't start our TX before the sender
   * has finished its own.
   */
  fwd_delay = BMRF_FWD_DELAY();

  /* Finalise D: D = min(BMRF_FWD_DELAY(), BMRF_MIN_FWD_DELAY) */
#if BMRF_MIN_FWD_DELAY
  if(fwd_delay < BMRF_MIN_FWD_DELAY) {
    fwd_delay = BMRF_MIN_FWD_DELAY;
  }
#endif

  if(fwd_delay == 0) {
    /* No delay required, send it, do it now, why wait? */
    UIP_IP_BUF->ttl--;
    tcpip_output(NULL);
    UIP_IP_BUF->ttl++;        /* Restore before potential upstack delivery */
  } else {
    /* Randomise final delay in [D , D*Spread], step D */
    fwd_spread = BMRF_INTERVAL_COUNT;
    if(fwd_spread > BMRF_MAX_SPREAD) {
      fwd_spread = BMRF_MAX_SPREAD;
    }
    if(fwd_spread) {
      fwd_delay = fwd_delay * (1 + ((random_rand() >> 11) % fwd_spread));
    }

    memcpy(&mcast_buf, uip_buf, uip_len);
    mcast_len = uip_len;
    ctimer_set(&mcast_periodic, fwd_delay, mcast_fwd, NULL);
  }
  PRINTF("BMRF: %u bytes: fwd in %u [%u]\n",
         uip_len, fwd_delay, fwd_spread);
}
static void
mcast_fwd_with_unicast(void *p)
{
  uip_mcast6_route_t *mcast_entries;
  mcast_entries = NULL;
  for(mcast_entries = uip_mcast6_route_list_head();
      mcast_entries != NULL;
      mcast_entries = list_item_next(mcast_entries)) {
    if(uip_ipaddr_cmp(&mcast_entries->group, &UIP_IP_BUF->destipaddr)) {
      //Send to this address &mcast_entries->subscribed_child
      tcpip_output(&mcast_entries->subscribed_child);
    }
  }
}
static void
mcast_fwd_with_unicast_up_down(uip_lladdr_t *preferred_parent)
{
  uip_mcast6_route_t *mcast_entries;
  mcast_entries = NULL;
  for(mcast_entries = uip_mcast6_route_list_head();
      mcast_entries != NULL;
      mcast_entries = list_item_next(mcast_entries)) {
    if(uip_ipaddr_cmp(&mcast_entries->group, &UIP_IP_BUF->destipaddr)
      && !linkaddr_cmp(&mcast_entries->subscribed_child, packetbuf_addr(PACKETBUF_ADDR_SENDER))) {
      //Send to this address &mcast_entries->subscribed_child
      tcpip_output(&mcast_entries->subscribed_child);
    }
  }
  //Send to our preferred parent address preferred_parent
  tcpip_output(&preferred_parent);
}
/*---------------------------------------------------------------------------*/
static uint8_t
in()
{
  rpl_dag_t *d;                       /* Our DODAG */
  uip_ipaddr_t *parent_ipaddr;        /* Our pref. parent's IPv6 address */
  const uip_lladdr_t *parent_lladdr;  /* Our pref. parent's LL address */
#if BMRF_MODE == BMRF_MIXED_MODE
  uip_mcast6_route_t *locmcastrt;
  uint8_t entries_number;
#endif

  /*
   * Fetch a pointer to the LL address of our preferred parent
   */
  if(UIP_IP_BUF->proto == UIP_PROTO_HBHO && UIP_HBHO_BUF->len == RPL_HOP_BY_HOP_LEN - 8) {
    d = ((rpl_instance_t *)rpl_get_instance(UIP_EXT_HDR_OPT_RPL_BUF->instance))->current_dag;
  }

  if(!d) {
    d = rpl_get_any_dag();
  }

  if(!d) {
    UIP_MCAST6_STATS_ADD(mcast_dropped);
    return UIP_MCAST6_DROP;
  }

  /* Retrieve our preferred parent's LL address */
  parent_ipaddr = rpl_get_parent_ipaddr(d->preferred_parent);
  parent_lladdr = uip_ds6_nbr_lladdr_from_ipaddr(parent_ipaddr);

  if(parent_lladdr == NULL) {
    UIP_MCAST6_STATS_ADD(mcast_dropped);
    return UIP_MCAST6_DROP;
  }

  if(UIP_IP_BUF->ttl <= 1) {
    UIP_MCAST6_STATS_ADD(mcast_dropped);
    return UIP_MCAST6_DROP;
  }

  /* LL Broadcast or LL Unicast from above*/
  if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),&linkaddr_null)
    || linkaddr_cmp(parent_lladdr, packetbuf_addr(PACKETBUF_ADDR_SENDER))) {
    /*
     * We accept a datagram if it arrived from our preferred parent, discard
     * otherwise.
     */
    if(linkaddr_cmp(parent_lladdr, packetbuf_addr(PACKETBUF_ADDR_SENDER))) {
      PRINTF("BMRF: Routable in but BMRF ignored it\n");
      UIP_MCAST6_STATS_ADD(mcast_dropped);
      return UIP_MCAST6_DROP;
    }

    /* If we have an entry in the mcast routing table, something with
     * a higher RPL rank (somewhere down the tree) is a group member */
    if(!uip_mcast6_route_lookup(&UIP_IP_BUF->destipaddr)) {
      UIP_MCAST6_STATS_ADD(mcast_fwd);
#if BMRF_MODE == BMRF_UNICAST_MODE
      mcast_fwd_with_unicast_down();
#elif BMRF_MODE == BMRF_BROADCAST_MODE
      mcast_fwd_with_broadcast();
#elif BMRF_MODE == BMRF_MIXED_MODE
      locmcastrt = NULL;
      entries_number = 0;
      for(locmcastrt = uip_mcast6_route_list_head();
          locmcastrt != NULL;
          locmcastrt = list_item_next(locmcastrt)) {
        if(uip_ipaddr_cmp(&locmcastrt->group, &UIP_IP_BUF->destipaddr) && ++entries_number > BMRF_BROADCAST_THRESHOLD) {
          break;
        }
      }
      if(entries_number > BMRF_BROADCAST_MODE) {
        mcast_fwd_with_broadcast();
      } else {
        mcast_fwd_with_unicast();
      }
#endif /* BMRF_MODE */
    }
  } else {
    uip_ipaddr_t *ll_sender_ip_address;
    ll_sender_ip_address = uip_ds6_nbr_ipaddr_from_lladdr(packetbuf_addr(PACKETBUF_ADDR_SENDER));

    /* Unicast from below */
    if (ll_sender_ip_address != NULL && uip_ds6_route_lookup(ll_sender_ip_address)) {
      /* If we enter here, we will definitely forward */
      UIP_MCAST6_STATS_ADD(mcast_fwd);
      mcast_fwd_with_unicast_up_down(&parent_lladdr);
    } else {
      UIP_MCAST6_STATS_ADD(mcast_dropped);
      return UIP_MCAST6_DROP;
    }
  }

  UIP_MCAST6_STATS_ADD(mcast_in_all);
  UIP_MCAST6_STATS_ADD(mcast_in_unique);

  /* Done with this packet unless we are a member of the mcast group */
  if(!uip_ds6_is_my_maddr(&UIP_IP_BUF->destipaddr)) {
    PRINTF("BMRF: Not a group member. No further processing\n");
    return UIP_MCAST6_DROP;
  } else {
    PRINTF("BMRF: Ours. Deliver to upper layers\n");
    UIP_MCAST6_STATS_ADD(mcast_in_ours);
    return UIP_MCAST6_ACCEPT;
  }
}
/*---------------------------------------------------------------------------*/
static void
init()
{
  UIP_MCAST6_STATS_INIT(NULL);

  uip_mcast6_route_init();
}
/*---------------------------------------------------------------------------*/
static void
out()
{
  UIP_MCAST6_STATS_ADD(mcast_out);
  return;
}
/*---------------------------------------------------------------------------*/
const struct uip_mcast6_driver bmrf_driver = {
  "BMRF",
  init,
  out,
  in,
};
/*---------------------------------------------------------------------------*/

#endif /* UIP_CONF_IPV6 */
