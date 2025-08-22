/*******************************************************************************
**** 文件路径         : \GD32_FreeRTOS_Lwip\Bsp\inc\enet.h
**** 作者名称         : Amosico
**** 文件版本         : V1.0.0
**** 创建日期         : 2025-04-28 15:49:52
**** 简要说明         : 
**** 版权信息         :
********************************************************************************/

#ifndef _ENET_H
#define _ENET_H

#include "gd32h7xx.h"


/* MAC Addr */
#define BOARD_MAC_ADDR0 2
#define BOARD_MAC_ADDR1 0xA
#define BOARD_MAC_ADDR2 0xF
#define BOARD_MAC_ADDR3 0xE
#define BOARD_MAC_ADDR4 0xD
#define BOARD_MAC_ADDR5 6
/* Board IP Addr */
#define BOARD_IP_ADDR0 192
#define BOARD_IP_ADDR1 168
#define BOARD_IP_ADDR2 57
#define BOARD_IP_ADDR3 100
/* Remote Device IP Addr */
#define DEVICE_IP_ADDR0 192
#define DEVICE_IP_ADDR1 168
#define DEVICE_IP_ADDR2 57
#define DEVICE_IP_ADDR3 158
/* IP Mask */
#define BOARD_NETMASK_ADDR0 255
#define BOARD_NETMASK_ADDR1 255
#define BOARD_NETMASK_ADDR2 255
#define BOARD_NETMASK_ADDR3 0
/* IP GW */
#define BOARD_GW_ADDR0 192
#define BOARD_GW_ADDR1 168
#define BOARD_GW_ADDR2 57
#define BOARD_GW_ADDR3 1

ErrStatus usr_enet_init(void);

#endif
