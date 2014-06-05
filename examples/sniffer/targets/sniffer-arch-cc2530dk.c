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
 *     Arch-specific sniffer functionality for the cc2530
 *
 * \author
 *     George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "contiki-conf.h"
#include "dev/io-arch.h"
#include "dev/cc2530-rf.h"
#include "cc253x.h"
#include "sniffer.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#if SNIFFER_SENSNIFF_SUPPORT
void
sniffer_arch_set_input(int (* input)(unsigned char c))
{
  io_arch_set_input(input);
}
#endif
/*---------------------------------------------------------------------------*/
void
sniffer_arch_write_byte(uint8_t data)
{
  io_arch_writeb(data);
}
/*---------------------------------------------------------------------------*/
void
sniffer_arch_flush_ouput(void)
{
  io_arch_flush();
}
/*---------------------------------------------------------------------------*/
void
sniffer_arch_output_frame(uint8_t *frame, uint8_t len)
{
  uint8_t i;

  for(i = 0; i < len; i++) {
    io_arch_writeb(frame[i]);
  }
  io_arch_flush();
}
/*---------------------------------------------------------------------------*/
uint8_t
sniffer_arch_rf_get_channel(void)
{
  return cc2530_rf_channel_get();
}
/*---------------------------------------------------------------------------*/
uint8_t
sniffer_arch_rf_set_channel(uint8_t channel)
{
  if((channel >= 11) && (channel <= 26)) {
    cc2530_rf_channel_set(channel);
  }
  return cc2530_rf_channel_get();
}
/*---------------------------------------------------------------------------*/
void
sniffer_arch_init()
{
  /* Turn off address recognition / frame filtering  */
  FRMFILT0 &= ~0x01;
}
/*---------------------------------------------------------------------------*/
