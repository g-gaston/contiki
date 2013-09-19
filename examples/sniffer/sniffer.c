/*
 * Copyright (c) 2013, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *     Example sniffer for IEEE 802.15.4 networks
 *
 *     This example is to be used combined with the sensniff host-side tool.
 *     You can download sensniff from: https://github.com/g-oikonomou/sensniff
 *
 * \author
 *     George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "contiki.h"
#include "sys/timer.h"
#include "sniffer.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"
#include "debug.h"
#include <string.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* State machine constants and variables */
#define STATE_WAITING_FOR_MAGIC 0
#define STATE_WAITING_FOR_VER   1
#define STATE_WAITING_FOR_CMD   2
#define STATE_WAITING_FOR_LEN   3
#define STATE_READING_DATA      4
/*---------------------------------------------------------------------------*/
PROCESS(sniffer_process, "Sniffer process");
AUTOSTART_PROCESSES(&sniffer_process);
/*---------------------------------------------------------------------------*/
#define SNIFFER_CMD_TIMEOUT (CLOCK_SECOND >> 1)
/*---------------------------------------------------------------------------*/
#define SNIFFER_CMD_LEN_MAX     4
#define SNIFFER_MAGIC { 0xC1, 0x1F, 0xFE, 0x72 }

struct sniffer_cmd {
  uint8_t magic[4];
  uint8_t ver;
  uint8_t cmd;
  uint8_t len;
  uint8_t buf[SNIFFER_CMD_LEN_MAX];
};
/*---------------------------------------------------------------------------*/
#if SNIFFER_SENSNIFF_SUPPORT
static volatile uint8_t loc;
static volatile uint8_t state;
static const uint8_t magic[] = SNIFFER_MAGIC;
static volatile struct sniffer_cmd cmd;
/*---------------------------------------------------------------------------*/
/*
 * We maintain a timer while receiving a command frame. If frame reception does
 * not complete before the timeout, trash the frame
 */
static struct ctimer ct;
/*---------------------------------------------------------------------------*/
static void
trash_frame(void *ptr)
{
  memset(&cmd, 0, sizeof(cmd));
  loc = 0;
  state = STATE_WAITING_FOR_MAGIC;
  ctimer_stop(&ct);
}
/*---------------------------------------------------------------------------*/
static void
input_byte(unsigned char c)
{
  uint8_t more = 1;

  switch(state) {
  case STATE_WAITING_FOR_MAGIC:
    if(c == magic[loc]) {
      cmd.magic[loc] = c;
      loc++;
    } else {
      PRINTF("Input error: [0x%02x - 0x%02x]\n", magic[loc], c);
      trash_frame(NULL);
      return;
    }

    if(loc == sizeof(cmd.magic)) {
      PRINTF("Input: MAGIC OK %u\n", loc);
      loc = 0;
      if(memcmp(magic, cmd.magic, sizeof(magic)) == 0) {
        PRINTF("Input: MAGIC OK\n");
        state = STATE_WAITING_FOR_VER;
      } else {
        /* Should never enter this branch */
        trash_frame(NULL);
        return;
      }
    }
    break;
  case STATE_WAITING_FOR_VER:
    cmd.ver = c;
    if(cmd.ver == SNIFFER_PROTO_VERSION) {
      state = STATE_WAITING_FOR_CMD;
    } else {
      PRINTF("Input error (STATE_WAITING_FOR_VER): [0x%02x]\n", c);
      trash_frame(NULL);
      return;
    }
    break;
  case STATE_WAITING_FOR_CMD:
    cmd.cmd = c;
    if(cmd.cmd == SNIFFER_CMD_GET_CHANNEL) {
      state = STATE_WAITING_FOR_MAGIC;
      more = 0;
    } else if(cmd.cmd == SNIFFER_CMD_SET_CHANNEL) {
      state = STATE_WAITING_FOR_LEN;
    } else {
      PRINTF("Input error (STATE_WAITING_FOR_CMD): [0x%02x]\n", c);
      trash_frame(NULL);
    }
    break;
  case STATE_WAITING_FOR_LEN:
    cmd.len = c;
    if(cmd.len > SNIFFER_CMD_LEN_MAX) {
      /* Too Long */
      PRINTF("Input error (STATE_WAITING_FOR_LEN): [0x%02x]\n", cmd.len);
      trash_frame(NULL);
    }
    state = STATE_READING_DATA;
    break;
  case STATE_READING_DATA:
    cmd.buf[loc] = c;
    loc++;

    if(loc == cmd.len) {
      more = 0;
    }
    break;
  }

  if(more) {
    ctimer_set(&ct, SNIFFER_CMD_TIMEOUT, trash_frame, NULL);
  } else {
    loc = 0;
    process_poll(&sniffer_process);
    ctimer_stop(&ct);
  }

  return;
}
/*---------------------------------------------------------------------------*/
void
sniffer_send_command(uint8_t cmd, uint8_t len, uint8_t *buf)
{
  int i;

  for(i = 0; i < sizeof(magic); i++) {
    sniffer_arch_write_byte(magic[i]);
  }

  sniffer_arch_write_byte(SNIFFER_PROTO_VERSION);
  sniffer_arch_write_byte(cmd);
  sniffer_arch_write_byte(len);

  for(i = 0; i < len; i++) {
    sniffer_arch_write_byte(buf[i]);
  }

  sniffer_arch_flush_ouput();

#if DEBUG
  PRINTF("sniffer: Out [ ");

  for(i = 0; i < sizeof(magic); i++) {
    PRINTF("%02x ", magic[i]);
  }

  PRINTF("%02x ", SNIFFER_PROTO_VERSION);
  PRINTF("%02x ", cmd);
  PRINTF("%02x ", len);

  for(i = 0; i < len; i++) {
    PRINTF("%02x ", buf[i]);
  }

  PRINTF("]\n");
#endif
}
#endif /* SNIFFER_SENSNIFF_SUPPORT */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sniffer_process, ev, data)
{

  PROCESS_BEGIN();

  PRINTF("sniffer started\n");

  sniffer_arch_init();

  /*
   * If we do not want to support commands from the host, we are done here.
   * If we do, hook in the input callback and wait for byte input events.
   */

#if SNIFFER_SENSNIFF_SUPPORT
  loc = 0;
  state = STATE_WAITING_FOR_MAGIC;

  sniffer_arch_set_input((int (*)(unsigned char c))input_byte);

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_POLL) {
      PRINTF("Polled\n");
      if(cmd.cmd == SNIFFER_CMD_GET_CHANNEL
         || cmd.cmd == SNIFFER_CMD_SET_CHANNEL) {
        PRINTF("sniffer: Command 0x%02x\n", cmd.cmd);

        if(cmd.cmd == SNIFFER_CMD_SET_CHANNEL) {
          PRINTF("sniffer: SET_CHANNEL command\n");
          sniffer_arch_rf_set_channel(cmd.buf[0]);
        }

        cmd.buf[0] = sniffer_arch_rf_get_channel();

        PRINTF("sniffer: Channel %u\n", cmd.buf[0]);

        sniffer_send_command(SNIFFER_CMD_CHANNEL, 1, cmd.buf);

        /* Processed. Clear buffers, restore state and prepare for next */
        trash_frame(NULL);
      }
    }
  }
#endif

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
