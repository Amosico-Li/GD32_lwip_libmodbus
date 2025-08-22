/*******************************************************************************
**** 文件路径         : \GD32_FreeRTOS_Lwip_libmodbus\App\src\app_task.c
**** 作者名称         : Amosico
**** 文件版本         : V1.0.0
**** 创建日期         : 2025-04-09 16:10:50
**** 简要说明         :
**** 版权信息         :
********************************************************************************/

/* LWIP */
#include "arch/sys_arch.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "port/ethernetif.h"
#include "sys/socket.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
/* Peripherals */
#include "enet.h"
/* libmodbus */
#include "modbus.h"

/* LWIP Stack Init*/

struct netif g_mynetif0;

void lwip_stack_init(void) {
    ip_addr_t gd_ipaddr;
    ip_addr_t gd_netmask;
    ip_addr_t gd_gw;

    /* create tcp_ip stack thread */
    tcpip_init(NULL, NULL);

    /* IP address setting */
    IP4_ADDR(&gd_ipaddr, BOARD_IP_ADDR0, BOARD_IP_ADDR1, BOARD_IP_ADDR2, BOARD_IP_ADDR3);
    IP4_ADDR(&gd_netmask,
             BOARD_NETMASK_ADDR0,
             BOARD_NETMASK_ADDR1,
             BOARD_NETMASK_ADDR2,
             BOARD_NETMASK_ADDR3);
    IP4_ADDR(&gd_gw, BOARD_GW_ADDR0, BOARD_GW_ADDR1, BOARD_GW_ADDR2, BOARD_GW_ADDR3);


    /* add a new network interface */
    netif_add(&g_mynetif0, &gd_ipaddr, &gd_netmask, &gd_gw, NULL, &ethernetif_init, &tcpip_input);

    /* set a default network interface */
    netif_set_default(&g_mynetif0);

    /* set a callback when interface is up/down */
    // netif_set_status_callback(&g_mynetif0, lwip_netif_status_callback);

    /* set the flag of netif as NETIF_FLAG_LINK_UP */
    netif_set_link_up(&g_mynetif0);

    /* bring an interface up and set the flag of netif as NETIF_FLAG_UP */
    netif_set_up(&g_mynetif0);
}


/* FreeRTOS User Part*/
// IP Stack Tsk Init
#define INIT_TASK_PRIO       30
#define LED_TASK_PRIO        20
#define TCP_CLIENT_TASK_PRIO 24
#define TCP_SERVER_TASK_PRIO 24
static void Tcp_Client_Tsk_Init(void);
static void Tcp_Server_Tsk_Init(void);

static TaskHandle_t init_tsk_handle = NULL;
static TaskHandle_t led_tsk_handle = NULL;

void led_spark_ms(uint32_t inv_ms) {
    gpio_bit_toggle(GPIOA, GPIO_PIN_6);
    vTaskDelay(inv_ms);
}
static void led_tsk(void* para) {
    for(;;) {
        led_spark_ms(500);
    }
}

static void init_tsk(void* para) {

    lwip_stack_init();

    //Tcp_Client_Tsk_Init();        //Success
    Tcp_Server_Tsk_Init();
    uint8_t xReturn = xTaskCreate((TaskFunction_t)led_tsk,
                                  (const char*)"led",
                                  (uint16_t)128,
                                  (void*)NULL,
                                  (UBaseType_t)LED_TASK_PRIO,
                                  (TaskHandle_t*)&led_tsk_handle);
    vTaskDelete(init_tsk_handle);
}


uint8_t FreeRTOS_Task_Create_And_Start(void) {
    uint8_t xReturn = xTaskCreate((TaskFunction_t)init_tsk,
                                  (const char*)"init",
                                  (uint16_t)512,
                                  (void*)NULL,
                                  (UBaseType_t)INIT_TASK_PRIO,
                                  (TaskHandle_t*)&init_tsk_handle);

    if(pdTRUE == xReturn)
        vTaskStartScheduler();

    return xReturn;
}


/* Modbus-Tcp-Client */
static void TCP_Client_Tsk(void* para) {
    int ret;
    modbus_t* ctx = NULL;
    u16_t tab_reg[64] = {0};

    for(;;) {
        ctx = modbus_new_tcp("192.168.57.158", 49936);
        if(ctx == NULL) {
            continue;
        }
        modbus_set_debug(ctx, 1);
        modbus_set_slave(ctx, 1);
        modbus_set_byte_timeout(ctx, 0, 500000);
        modbus_set_response_timeout(ctx, 0, 500000);
        modbus_flush(ctx);

        ret = modbus_connect(ctx);
        if(ret == -1) {
            printf("Modbus connect failed!\n");
            modbus_free(ctx);
            ctx = NULL;
            continue;
        }

        while(ctx != NULL) {
            /* 0x03 */
            int rc = modbus_read_registers(ctx, 0, 3, tab_reg);
            if(rc == -1) {
                printf("read error!\n");
            } else {
                for(int i = 0; i < rc; i++) {
                    printf("reg[%d]=%d(0x%X)\n", i, tab_reg[i], tab_reg[i]);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        modbus_close(ctx);
        modbus_free(ctx);
        ctx = NULL;
    }
}

void Tcp_Client_Tsk_Init(void) {
    sys_thread_new("TCP_Client", (lwip_thread_fn)TCP_Client_Tsk, NULL, 512, TCP_CLIENT_TASK_PRIO);
}


/* Modbus-Tcp-Server */
#define BUF_SIZE 50
static void TCP_Server_Tsk(void* para) {
    int rc, newfd = -1, sockfd = -1;
    char ip[16] = "192.168.57.100";
    modbus_t* ctx = NULL;
    modbus_mapping_t* mb_mapping = NULL;
    u8_t recvbuf[BUF_SIZE] = {0};
Crate_New_Tcp: {
    ctx = modbus_new_tcp(&ip[0], 49936);
    if(ctx == NULL) {
        printf("App: Create new tcp server Failed!\n");
        goto Crate_New_Tcp;
    }
    printf("App: Create tcp port Success!\n");
    modbus_set_debug(ctx, 1);
    modbus_set_slave(ctx, 1);
    modbus_set_byte_timeout(ctx, 0, 500000);
    modbus_set_response_timeout(ctx, 0, 500000);
    modbus_flush(ctx);
}
Create_Modbus_MAp: {
    mb_mapping = modbus_mapping_new(5, 5, 5, 5);
    if(mb_mapping == NULL) {
        printf("App: Create modbus map Failed!\n");
        goto Create_Modbus_MAp;
    }
}
    // Initialize modbus map -> 0x03 Register
    mb_mapping->start_registers = 0;
    for(int i = 0; i < 5; ++i) {
        mb_mapping->tab_registers[mb_mapping->start_registers + i] = i;
    }
    printf("App: Initialize mb map Success!\n");

    sockfd = modbus_tcp_listen(ctx, 1);
    if(sockfd < 0) {
        printf("App: Listen Failed!\n");
        goto Modbus_End;
    }
    newfd = modbus_tcp_accept(ctx, &sockfd);
    if(newfd == -1) {
        printf("App: Accept Failed!\n");
        goto Modbus_End;
    }
    printf("App: Conn set up Success!\n");

    while(newfd > 0) {
        rc = modbus_receive(ctx, recvbuf);
        if(rc > 0) {
            modbus_reply(ctx, recvbuf, rc, mb_mapping);
        } else if(rc == -1) {
            printf("App: Recv error!\n");
        }
    }
Modbus_End: {
    modbus_close(ctx);
    modbus_free(ctx);
    ctx = NULL;
    modbus_mapping_free(mb_mapping);
    mb_mapping = NULL;
    printf("App: Modbus Free success!\n");
    goto Crate_New_Tcp;
}
}

void Tcp_Server_Tsk_Init(void) {
    sys_thread_new("TCP_Server", (lwip_thread_fn)TCP_Server_Tsk, NULL, 512, TCP_SERVER_TASK_PRIO);
}
