/*******************************************************************************
**** 文件路径         : \GD32_FreeRTOS_Lwip\Bsp\src\led.c
**** 作者名称         : Amosico
**** 文件版本         : V1.0.0
**** 创建日期         : 2025-06-09 14:39:51
**** 简要说明         :
**** 版权信息         :
********************************************************************************/

#include "gd32h7xx.h"


void led_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_bit_reset(GPIOA, GPIO_PIN_6);
}



