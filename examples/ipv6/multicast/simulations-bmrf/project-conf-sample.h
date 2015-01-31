/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 *         Project specific configuration defines for the RPl multicast simulations
 *
 * \author
 *         Guillermo Gastón
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#include "net/ipv6/multicast/uip-mcast6-engines.h"

/* Change this to switch engines. Engine codes in uip-mcast6-engines.h */
#define UIP_MCAST6_CONF_ENGINE	#engine#
#define BMRF_CONF_MODE	#mode#
#define BMRF_CONF_BROADCAST_THRESHOLD	#threshold#

/* For Imin: Use 16 over NullRDC, 64 over Contiki MAC */
#define ROLL_TM_CONF_IMIN_1         64

#undef UIP_CONF_IPV6_RPL
#undef UIP_CONF_ND6_SEND_RA
#undef UIP_CONF_ROUTER
#define UIP_CONF_IPV6_RPL            1
#define UIP_CONF_ND6_SEND_RA         0
#define UIP_CONF_ROUTER              1
#define UIP_MCAST6_ROUTE_CONF_ROUTES 8
#define CSMA_CONF_MAX_NEIGHBOR_QUEUES UIP_MCAST6_ROUTE_CONF_ROUTES
//#define QUEUEBUF_CONF_NUM 10

#undef UIP_CONF_TCP
#define UIP_CONF_TCP 0

/* Code/RAM footprint savings so that things will fit on our device */
#undef UIP_CONF_DS6_NBR_NBU
#undef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_NBR_NBU        10
#define UIP_CONF_DS6_ROUTE_NBU      10

/* Enable simulation stats */
#define SIMSTATS_CONF_ENABLED 1

/* Simulation parameters */
#define MCAST_CONF_MESSAGES 100
#define MCAST_CONF_SEND_INTERVAL 3
#define MCAST_CONF_START_DELAY 420

#endif /* PROJECT_CONF_H_ */
