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
 */
/**
 * \file
 *         RDC driver for Sniffers. Instead of pushing captured frames upwards
 *         in the network stack, it outputs them over a peripheral.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "net/mac/mac.h"
#include "net/mac/rdc.h"
#include "net/packetbuf.h"
#include "sniffer.h"

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define SNIFFER_DATA_LEN_MAX        128
#define CRC_OK                     0x80

static uint8_t frame_buff[SNIFFER_DATA_LEN_MAX];
/*---------------------------------------------------------------------------*/
static void
send(mac_callback_t sent, void *ptr)
{
  if(sent) {
    sent(ptr, MAC_TX_OK, 1);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *list)
{
  if(sent) {
    sent(ptr, MAC_TX_OK, 1);
  }
}
/*---------------------------------------------------------------------------*/
static void
input(void)
{
  uint8_t len = (uint8_t)packetbuf_datalen();

  /*
   * Copy the frame from packetbuf to our TX buffer. This will be the layer 2
   * header + payload but not the FCS
   */
  memcpy(frame_buff, (uint8_t *)packetbuf_dataptr(), len);

  /*
   * Append the FCS:
   * - RSSI is in packetbuf
   * - LQI is there too but we need to OR the byte with 0x80 (CRC OK).
   *
   * CRC is definitely OK, otherwise the RF driver would not have delivered
   * the frame to us
   */
  frame_buff[len] = ((uint8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI));
  len++;

  frame_buff[len] = ((uint8_t)(packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY)
                     | CRC_OK));
  len++;

#if SNIFFER_SENSNIFF_SUPPORT
  sniffer_send_command(SNIFFER_CMD_FRAME, len, frame_buff);
#else
  sniffer_arch_output_frame(frame_buff, len);
#endif
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return keep_radio_on;
}
/*---------------------------------------------------------------------------*/
static unsigned short
cca(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  return;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver sniffer_rdc_driver = {
  "sniffer-rdc",
  init,
  send,
  send_list,
  input,
  on,
  off,
  cca,
};
/*---------------------------------------------------------------------------*/
