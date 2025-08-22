/*******************************************************************************
**** 文件路径         : \GD32_FreeRTOS_Lwip\App\src\main.c
**** 作者名称         : Amosico
**** 文件版本         : V1.0.0
**** 创建日期         : 2025-05-20 10:48:32
**** 简要说明         :
**** 版权信息         :
********************************************************************************/

#include "FreeRTOS.h"
#include "app_task.h"
#include "bsp.h"
#include "gd32h7xx.h"
#include "systick.h"
#include <stdio.h>

#include "enet.h"

void cache_enable(void);

// Fuctions
void periph_clk_init(void) {
    rcu_periph_clock_enable(RCU_SYSCFG);
}

void irq_init(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    nvic_irq_enable(USART0_IRQn, 6, 0);
}

void mpu_config(void) {
    mpu_region_init_struct mpu_init_struct;
    mpu_region_struct_para_init(&mpu_init_struct);

    /* disable the MPU */
    ARM_MPU_SetRegion(0U, 0U);

    /* Configure the DMA descriptors and Rx/Tx buffer*/
    mpu_init_struct.region_base_address = 0x30000000;
    mpu_init_struct.region_size = MPU_REGION_SIZE_16KB;
    mpu_init_struct.access_permission = MPU_AP_FULL_ACCESS;
    mpu_init_struct.access_bufferable = MPU_ACCESS_BUFFERABLE;
    mpu_init_struct.access_cacheable = MPU_ACCESS_NON_CACHEABLE;
    mpu_init_struct.access_shareable = MPU_ACCESS_NON_SHAREABLE;
    mpu_init_struct.region_number = MPU_REGION_NUMBER0;
    mpu_init_struct.subregion_disable = MPU_SUBREGION_ENABLE;
    mpu_init_struct.instruction_exec = MPU_INSTRUCTION_EXEC_PERMIT;
    mpu_init_struct.tex_type = MPU_TEX_TYPE0;
    mpu_region_config(&mpu_init_struct);
    mpu_region_enable();

    /* Configure the LwIP RAM heap */
    mpu_init_struct.region_base_address = 0x30004000;
    mpu_init_struct.region_size = MPU_REGION_SIZE_16KB;
    mpu_init_struct.access_permission = MPU_AP_FULL_ACCESS;
    mpu_init_struct.access_bufferable = MPU_ACCESS_NON_BUFFERABLE;
    mpu_init_struct.access_cacheable = MPU_ACCESS_NON_CACHEABLE;
    mpu_init_struct.access_shareable = MPU_ACCESS_SHAREABLE;
    mpu_init_struct.region_number = MPU_REGION_NUMBER1;
    mpu_init_struct.subregion_disable = MPU_SUBREGION_ENABLE;
    mpu_init_struct.instruction_exec = MPU_INSTRUCTION_EXEC_PERMIT;
    mpu_init_struct.tex_type = MPU_TEX_TYPE1;
    mpu_region_config(&mpu_init_struct);
    mpu_region_enable();

    /* enable the MPU */
    ARM_MPU_Enable(MPU_MODE_PRIV_DEFAULT);
}

int main(void) {
    /* enable the CPU cache */
    mpu_config();
    cache_enable();
    systick_config();

    periph_clk_init();

    gd_bsp_init();
    irq_init();

    uint8_t xReturn = FreeRTOS_Task_Create_And_Start();
    if(xReturn != pdTRUE) {
        printf("FreeRTOS Init Error!\n");
    }

    while(1) {}
}


/* retarget the C library printf function to the USART */
int fputc(int ch, FILE* f) {
    usart_data_transmit(USART0, (uint8_t)ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));

    return ch;
}

/*!
    \brief      enable the CPU cache
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cache_enable(void) {
    /* enable i-cache */
    SCB_EnableICache();

    /* enable d-cache */
    SCB_EnableDCache();
}
