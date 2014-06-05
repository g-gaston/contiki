/*
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
 *     Arch-specific sniffer functionality for the z1
 *
 * \author
 *     Guillermo Gastón
 */
#include "contiki-conf.h"
#include "cpu/msp430/dev/uart0.h"
#include "dev/cc2420/cc2420.h"
#include "netstack.h"
#include "sniffer.h"

#include <stdint.h>

#define AUTOACK (1 << 4)
#define ADR_DECODE (1 << 11)

/*---------------------------------------------------------------------------*/
#if SNIFFER_SENSNIFF_SUPPORT
void
sniffer_arch_set_input(int (* input)(unsigned char c))
{
  uart0_set_input(input);
}
#endif
/*---------------------------------------------------------------------------*/
void
sniffer_arch_write_byte(uint8_t data)
{
  uart0_writeb(data);
}
/*---------------------------------------------------------------------------*/
void
sniffer_arch_flush_ouput(void)
{
  sniffer_arch_write_byte(0300);
}
/*---------------------------------------------------------------------------*/
void
sniffer_arch_output_frame(uint8_t *frame, uint8_t len)
{
  uint8_t i;

  for(i = 0; i < len; i++) {
    sniffer_arch_write_byte(frame[i]);
  }
  sniffer_arch_flush_ouput();
}
/*---------------------------------------------------------------------------*/
uint8_t
sniffer_arch_rf_get_channel(void)
{
  return cc2420_get_channel();
}
/*---------------------------------------------------------------------------*/
uint8_t
sniffer_arch_rf_set_channel(uint8_t channel)
{
  if((channel >= 11) && (channel <= 26)) {
    cc2420_set_channel(channel);
  }
  return sniffer_arch_rf_get_channel();
}
/*---------------------------------------------------------------------------*/
void
sniffer_arch_init()
{
  uint16_t reg;

  NETSTACK_RADIO.on();

  /* Turn off address recognition / frame filtering  */
  CC2420_READ_REG(CC2420_MDMCTRL0, reg);
  reg &= ~(AUTOACK | ADR_DECODE);
  CC2420_WRITE_REG(CC2420_MDMCTRL0, reg);
}
/*---------------------------------------------------------------------------*/
