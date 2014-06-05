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
 *     Header file with macros and function prototypes for the sniffer
 *     abstraction
 *
 * \author
 *     George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#ifndef SNIFFER_H_
#define SNIFFER_H_

#include "contiki-conf.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* Sensniff Command and Response Codes */
#define SNIFFER_CMD_FRAME               0x00
#define SNIFFER_CMD_CHANNEL             0x01
#define SNIFFER_CMD_GET_CHANNEL         0x81
#define SNIFFER_CMD_SET_CHANNEL         0x82
#define SNIFFER_PROTO_VERSION              1
/*---------------------------------------------------------------------------*/
/* Sensniff support */
#ifdef SNIFFER_CONF_SENSNIFF_SUPPORT
#define SNIFFER_SENSNIFF_SUPPORT SNIFFER_CONF_SENSNIFF_SUPPORT
#else
#define SNIFFER_SENSNIFF_SUPPORT 0
#endif
/*---------------------------------------------------------------------------*/
void sniffer_send_command(uint8_t cmd, uint8_t len, uint8_t *buf);
/*---------------------------------------------------------------------------*/
/*
 * Arch-specific API
 * To support this example, a platform must provide an implementation of all
 * hardware-specific functionality in a file called sniffer-arch-$(TARGET).c
 * (e.g. sniffer-arch-sky.c)
 * The build system will add this file to the sources list automatically
 */

/**
 * \brief Hardware-specific init routine
 *
 * Typically, this is where address recongition / frame filtering and
 * generation of hardware auto-ACKs will be turned off
 */
void sniffer_arch_init(void);

/*
 * Character Input/Ouput.
 *
 * We need to be able to read commands sent by the host and received over our
 * peripheral. We need to be able to send captured frames and responses back to
 * the host, over the same peripheral.
 *
 * Rather than make assumptions on which peripheral is used by each platform
 * (UART vs USB and UART0 vs UART1), we provide an abstraction and delegate
 * implementation to the platform.
 */

/**
 * \brief Set a callback function to handle sniffer command input.
 * \param f A function to be called by the platform code when a byte is
 *          received from the host.
 *
 * This function will be called by the sniffer to register itself as the byte
 * input processing module. We use this so that host-originated commands can be
 * directed to the sniffer for processing,
 *
 * In its simplest form, a platform would implement this as a call to
 * uartN_set_input().
 */
void sniffer_arch_set_input(int (* input)(unsigned char c));

/**
 * \brief Output a frame over a platform-specified peripheral
 * \param frame a pointer to a buffer holding the frame
 * \param len The frame length
 *
 * This is that function used by the sniffer to output frames and command
 * responses. This could for example be done over UART or USB.
 *
 * If sensniff is in use, the buffer will hold the entire sensniff frame,
 * including headers
 */
void sniffer_arch_output_frame(uint8_t *frame, uint8_t len);

/**
 * \brief Get the current RF operating channel
 * \return The current operating channel in the range [11 , 26]
 */
uint8_t sniffer_arch_rf_get_channel(void);

/**
 * \brief Set the current RF operating channel
 * \param channel The desired new operating channel in the range [11 , 26]
 * \return The operating channel after the change in the range [11 , 26]
 *
 * For simplicity in the sniffer implementation, we avoid a signed return
 * datatype (whereby negative would signal failure). Failure can be indirectly
 * detected when \e channel != return value
 */
uint8_t sniffer_arch_rf_set_channel(uint8_t channel);

#endif /* SNIFFER_H_ */
