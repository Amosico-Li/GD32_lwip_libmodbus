/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_LWIPOPTS_H__
#define LWIP_HDR_LWIPOPTS_H__

#define LWIP_FUZZ_SYS_NOW
#define LWIP_RAND_FOR_FUZZ


#define ETHARP_TRUST_IP_MAC  0
#define ARP_QUEUEING         0
#define SYS_LIGHTWEIGHT_PROT 1
#define NO_SYS                  0
#define LWIP_TCPIP_CORE_LOCKING 0

/*  memory options  */
#define MEM_ALIGNMENT 4
#define MEM_SIZE (15 * 1024)
/* Relocate the LwIP RAM heap pointer */
#define LWIP_RAM_HEAP_POINTER (0x30004000)
#define MEMP_NUM_PBUF 100
#define MEMP_NUM_UDP_PCB 6
#define MEMP_NUM_TCP_PCB 10       /* the number of simulatenously active TCP connections */
#define MEMP_NUM_TCP_PCB_LISTEN 5 /* the number of listening TCP connections */
#define MEMP_NUM_TCP_SEG 20       /* the number of simultaneously queued TCP segments */
#define MEMP_NUM_SYS_TIMEOUT 10   /* the number of simulateously active timeouts */
#define MEMP_NUM_NETBUF 8         /* the number of struct netbufs */

/* Pbuf options */
#define PBUF_POOL_SIZE     40  /* the number of buffers in the pbuf pool */
#define PBUF_POOL_BUFSIZE  1514 /* the size of each pbuf in the pbuf pool */
#define IP_REASS_MAX_PBUFS 20   /* total maximum amount of pbufs waiting to be reassembled */

/* TCP options */
#define LWIP_TCP 1
#define TCP_TTL 255
#define TCP_QUEUE_OOSEQ 0
#define TCP_MSS (1500 - 40)
#define TCP_SND_BUF (2 * TCP_MSS)
#define TCP_SND_QUEUELEN ((4 * TCP_SND_BUF) / TCP_MSS)
#define TCP_WND (2 * TCP_MSS)

/* ICMP options */
#define LWIP_ICMP 1

/* DHCP options */
#define LWIP_DHCP 1

/* UDP options */
#define LWIP_UDP 1
#define UDP_TTL  255

/* statistics options */
#define LWIP_STATS         0
#define LWIP_PROVIDE_ERRNO 1

/* checksum options */
#define CHECKSUM_BY_HARDWARE    /* computing and verifying the IP, UDP, TCP and ICMP               \
                                   checksums by hardware */

/* sequential layer options */
#define LWIP_NETCONN 1     /* set to 1 to enable netconn API (require to use api_lib.c) */

#define MEMP_NUM_NETCONN 4 /* the number of struct netconns */

/* socket options */
#define LWIP_SOCKET 1 /* set to 1 to enable socket API (require to use sockets.c) */
#define LWIP_SO_RCVTIMEO 1

/* Lwip debug options */
#define LWIP_DEBUG 1


#ifdef CHECKSUM_BY_HARDWARE
    /* CHECKSUM_GEN_IP==0: generate checksums by hardware for outgoing IP packets.*/
    #define CHECKSUM_GEN_IP 0
    /* CHECKSUM_GEN_UDP==0: generate checksums by hardware for outgoing UDP packets.*/
    #define CHECKSUM_GEN_UDP 0
    /* CHECKSUM_GEN_TCP==0: generate checksums by hardware for outgoing TCP packets.*/
    #define CHECKSUM_GEN_TCP 0
    /* CHECKSUM_CHECK_IP==0: check checksums by hardware for incoming IP packets.*/
    #define CHECKSUM_CHECK_IP 0
    /* CHECKSUM_CHECK_UDP==0: check checksums by hardware for incoming UDP packets.*/
    #define CHECKSUM_CHECK_UDP 0
    /* CHECKSUM_CHECK_TCP==0: check checksums by hardware for incoming TCP packets.*/
    #define CHECKSUM_CHECK_TCP 0
    #define CHECKSUM_GEN_ICMP  0
#else
    /* CHECKSUM_GEN_IP==1: generate checksums in software for outgoing IP packets.*/
    #define CHECKSUM_GEN_IP    1
    /* CHECKSUM_GEN_UDP==1: generate checksums in software for outgoing UDP packets.*/
    #define CHECKSUM_GEN_UDP   1
    /* CHECKSUM_GEN_TCP==1: generate checksums in software for outgoing TCP packets.*/
    #define CHECKSUM_GEN_TCP   1
    /* CHECKSUM_CHECK_IP==1: check checksums in software for incoming IP packets.*/
    #define CHECKSUM_CHECK_IP  1
    /* CHECKSUM_CHECK_UDP==1: check checksums in software for incoming UDP packets.*/
    #define CHECKSUM_CHECK_UDP 1
    /* CHECKSUM_CHECK_TCP==1: check checksums in software for incoming TCP packets.*/
    #define CHECKSUM_CHECK_TCP 1
    #define CHECKSUM_GEN_ICMP  1
#endif


/*
   ---------------------------------
   ---------- OS options ----------
   ---------------------------------
*/
#include "FreeRTOSConfig.h"

#define TCPIP_THREAD_NAME         "TCP/IP"
#define TCPIP_THREAD_STACKSIZE    1000
#define TCPIP_MBOX_SIZE           5
#define DEFAULT_THREAD_STACKSIZE  500
#define TCPIP_THREAD_PRIO         (configMAX_PRIORITIES - 5)
#define LWIP_COMPAT_MUTEX         1
#define DEFAULT_TCP_RECVMBOX_SIZE 8
#define DEFAULT_UDP_RECVMBOX_SIZE 8
#define DEFAULT_ACCEPTMBOX_SIZE   8


#endif /* LWIP_HDR_LWIPOPTS_H__ */
