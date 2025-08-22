/*******************************************************************************
**** 文件路径         : \GD32_FreeRTOS_Template\Bsp\src\usart.c
**** 作者名称         : Amosico
**** 文件版本         : V1.0.0
**** 创建日期         : 2025-04-09 16:13:49
**** 简要说明         :
**** 版权信息         :
********************************************************************************/

#include "gd32h7xx_usart.h"

/*USART*/
void usart_init(void) {
    // USART0
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);
    // USART0-GPIO
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_10);

    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_parity_config(USART0, USART_PM_NONE);

    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_enable(USART0);
}

// Normal transmit
// uint8_t usart_transmit(uint32_t usart_periph, uint8_t* pd, uint32_t len) {
//     uint8_t timeout = 3;
//     for(int i = 0; i < len; i++) {
//         usart_data_transmit(USART0, pd[i]);
//         while(usart_flag_get(USART0, USART_FLAG_TBE) == RESET) {
//             delay_1ms(100);
//             if(timeout-- < 0)
//                 return 1;
//         }
//     }
//     return 0;
// }

// USART IRQN
void USART0_IRQHandler(void) {
    if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RT) != RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RT);
    }
}
