/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/sys.h"

#include "FreeRTOS.h"
#include "enet.h"
#include "ethernetif.h"
#include "semphr.h"
#include "task.h"

#include <string.h>

#if 1 /* don't build, this is only a skeleton, see previous comment */

    #include "lwip/def.h"
    #include "lwip/etharp.h"
    #include "lwip/ethip6.h"
    #include "lwip/mem.h"
    #include "lwip/pbuf.h"
    #include "lwip/snmp.h"
    #include "lwip/stats.h"
    #include "netif/ppp/pppoe.h"

    /* Define those to better describe your network interface. */
    #define IFNAME0 'G'
    #define IFNAME1 'D'

/* Rx Semaphore Define(ethernetif_input) */
sys_sem_t r_xSemaphore = {.sem = NULL};

#define NETIF_IN_TASK_STACK_SIZE 256
#define NETIF_IN_TASK_PRIORITY   32

#include "gd32h7xx.h"
#include "gd32h7xx_enet.h"
extern enet_descriptors_struct rxdesc_tab[ENET_RXBUF_NUM], txdesc_tab[ENET_TXBUF_NUM];
extern uint8_t rx_buff[ENET_RXBUF_NUM][ENET_RXBUF_SIZE];
extern uint8_t tx_buff[ENET_TXBUF_NUM][ENET_TXBUF_SIZE];
extern enet_descriptors_struct* dma_current_txdesc;
extern enet_descriptors_struct* dma_current_rxdesc;

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
        struct eth_addr* ethaddr;
        /* Add whatever per-interface state that is needed here. */
};

/* Forward declarations. */
void ethernetif_input(struct netif* netif);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif* netif) {
    /* ENET Config */
    if(ERROR == usr_enet_init()) {
        printf("ENET INIT ERROR!\n");
    } else {
        printf("ENET INIT SUCCESS!\n");
    }

    struct ethernetif* ethernetif = netif->state;

    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    netif->hwaddr[0] = BOARD_MAC_ADDR0;
    netif->hwaddr[1] = BOARD_MAC_ADDR1;
    netif->hwaddr[2] = BOARD_MAC_ADDR2;
    netif->hwaddr[3] = BOARD_MAC_ADDR3;
    netif->hwaddr[4] = BOARD_MAC_ADDR4;
    netif->hwaddr[5] = BOARD_MAC_ADDR5;

    enet_mac_address_set(ENET0, ENET_MAC_ADDRESS0, netif->hwaddr);

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    enet_descriptors_chain_init(ENET0, ENET_DMA_TX);
    enet_descriptors_chain_init(ENET0, ENET_DMA_RX);

    int i;
    for(i = 0; i < ENET_RXBUF_NUM; i++) {
        enet_rx_desc_immediate_receive_complete_interrupt(&rxdesc_tab[i]);
    }
    for(i = 0; i < ENET_TXBUF_NUM; i++) {
        enet_transmit_checksum_config(&txdesc_tab[i], ENET_CHECKSUM_TCPUDPICMP_FULL);
    }

    #if LWIP_IPV6 && LWIP_IPV6_MLD
    /*
     * For hardware/netifs that implement MAC filtering.
     * All-nodes link-local is handled by default, so we must let the hardware know
     * to allow multicast packets in.
     * Should set mld_mac_filter previously. */
    if(netif->mld_mac_filter != NULL) {
        ip6_addr_t ip6_allnodes_ll;
        ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
        netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    }
    #endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

    /* Do whatever else is needed to initialize interface. */
    /* USER CODE BEGIN PHY_PRE_CONFIG */
    if(r_xSemaphore.sem == NULL) {
        sys_sem_new(&r_xSemaphore, 0);
    }

    /* create the task that handles the ETH_MAC */
    sys_thread_new("ETHIN",
                   (lwip_thread_fn)&ethernetif_input, /* 任务入口函数 */
                   netif,                            /* 任务入口函数参数 */
                   NETIF_IN_TASK_STACK_SIZE * 2,     /* 任务栈大小 */
                   NETIF_IN_TASK_PRIORITY - 2);      /* 任务的优先级 */

    enet_enable(ENET0);
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif* netif, struct pbuf* p) {
    struct ethernetif* ethernetif = netif->state;
    struct pbuf* q;
    ErrStatus err = ERROR;
    SYS_ARCH_DECL_PROTECT(sr);

    SYS_ARCH_PROTECT(sr);
    while((u32_t)RESET != (dma_current_txdesc->status & ENET_TDES0_DAV));
    u8_t* buffer =
        (u8_t*)(enet_desc_information_get(ENET0, dma_current_txdesc, TXDESC_BUFFER_1_ADDR));

    #if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
    #endif

    int framelength = 0;
    for(q = p; q != NULL; q = q->next) {
        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        memcpy((u8_t*)&buffer[framelength], q->payload, q->len);
        framelength = framelength + q->len;
    }

    err = ENET_NOCOPY_FRAME_TRANSMIT(ENET0, framelength);
    SYS_ARCH_UNPROTECT(sr);

    #if ETH_PAD_SIZE
    pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
    #endif
    LINK_STATS_INC(link.xmit);

    if(SUCCESS == err) {
        return ERR_OK;
    } else {
        while(1) {}
    }
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf* low_level_input(struct netif* netif) {
    struct ethernetif* ethernetif = netif->state;
    struct pbuf *p, *q;
    u16_t len;
    u8_t* buffer;
    /* Obtain the size of the packet and put it into the "len"
       variable. */
    len = enet_desc_information_get(ENET0, dma_current_rxdesc, RXDESC_FRAME_LENGTH);
    buffer = (u8_t*)(enet_desc_information_get(ENET0, dma_current_rxdesc, RXDESC_BUFFER_1_ADDR));

    #if ETH_PAD_SIZE
    len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
    #endif

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);


    if(p != NULL) {
    #if ETH_PAD_SIZE
        pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
    #endif
        int l = 0;
        for(q = p; q != NULL; q = q->next) {
            memcpy((uint8_t*)q->payload, (u8_t*)&buffer[l], q->len);
            l = l + q->len;
        }
    #if ETH_PAD_SIZE
        pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
    #endif
    }

    ENET_NOCOPY_FRAME_RECEIVE(ENET0);
    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
extern int flags;
void ethernetif_input(struct netif* netif) {
    struct ethernetif* ethernetif;
    // struct eth_hdr* ethhdr;
    struct pbuf* p = NULL;

    ethernetif = netif->state;

    for(;;) {
        if(sys_arch_sem_wait(&r_xSemaphore, 100)) {
            taskENTER_CRITICAL();
            /* move received packet into a new pbuf */
            p = low_level_input(netif);
            taskEXIT_CRITICAL();

            /* if no packet could be read, silently ignore this */
            if(p != NULL) {
                taskENTER_CRITICAL();
                /* pass all packets to ethernet_input, which decides what packets it supports */
                if(netif->input(p, netif) != ERR_OK) {
                    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                    pbuf_free(p);
                    p = NULL;
                }
                taskEXIT_CRITICAL();
            }
        }
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif* netif) {
    struct ethernetif* ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = mem_malloc(sizeof(struct ethernetif));
    if(ethernetif == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return ERR_MEM;
    }

    #if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
    #endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    // MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    #if LWIP_IPV4
    netif->output = etharp_output;
    #endif /* LWIP_IPV4 */
    #if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
    #endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;

    ethernetif->ethaddr = (struct eth_addr*)&(netif->hwaddr[0]);

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

#endif /* 0 */
