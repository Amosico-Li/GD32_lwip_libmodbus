/*******************************************************************************
**** 文件路径         : \GD32_FreeRTOS_Lwip\Bsp\src\enet.c
**** 作者名称         : Amosico
**** 文件版本         : V1.0.0
**** 创建日期         : 2025-04-28 15:49:46
**** 简要说明         :
**** 版权信息         :
********************************************************************************/

#include "gd32h7xx.h"

#include "ethernetif.h"
#include "lwip/sys.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"


static void enet_gpio_config(void) {

    /* PA8 -> CKOUT0 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOG);

    /* PA8: CKOUT0 */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_8);


    /* PA1: ETH0_RMII_REF_CLK */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_1);

    /* PA2: ETH0_MDIO */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_2);

    /* PA7: ETH0_RMII_CRS_DV */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_7);


    /* PG11: ETH0_RMII_TX_EN */
    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

    /* PB12: ETH0_RMII_TXD0 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_12);

    /* PG12: ETH0_RMII_TXD1 */
    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_12);


    /* PC1: ETH0_MDC */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_1);

    /* PC4: ETH0_RMII_RXD0 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_4);

    /* PC5: ETH0_RMII_RXD1 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_5);

    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_8);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_12);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_11);
    gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_12);

    rcu_ckout0_config(RCU_CKOUT0SRC_PLL0P, RCU_CKOUT0_DIV12);   // CKOUT0 = 50MHZ
    syscfg_enet_phy_interface_config(ENET0, SYSCFG_ENET_PHY_RMII);
}

static ErrStatus enet_dma_cofig(void) {
    ErrStatus err = SUCCESS;
    /* enable ethernet clock  */
    rcu_periph_clock_enable(RCU_ENET0);
    rcu_periph_clock_enable(RCU_ENET0TX);
    rcu_periph_clock_enable(RCU_ENET0RX);

    /* reset ethernet on AHB bus */
    enet_deinit(ENET0);

    err = enet_software_reset(ENET0);
    if(ERROR == err) {
        printf("ENET RESET ERROR!\n");
        return ERROR;
    }
    err = enet_init(ENET0,
                    ENET_AUTO_NEGOTIATION,
                    ENET_AUTOCHECKSUM_DROP_FAILFRAMES,
                    ENET_BROADCAST_FRAMES_PASS);
    return err;
}

ErrStatus usr_enet_init(void) {

    enet_gpio_config();

    ErrStatus es = ERROR;
    es = enet_dma_cofig();
    if(ERROR == es) {
        printf("ENET DMA CONFIG ERROR!\n");
        return ERROR;
    }

    nvic_irq_enable(ENET0_IRQn, 6, 0);
    enet_interrupt_enable(ENET0, ENET_DMA_INT_NIE);
    enet_interrupt_enable(ENET0, ENET_DMA_INT_RIE);

    return es;
}

int flags = 0;
extern sys_sem_t r_xSemaphore;
void ENET0_IRQHandler(void) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    /* frame received */
    if(SET == enet_interrupt_flag_get(ENET0, ENET_DMA_INT_FLAG_RS)) {
        /* give the semaphore to wakeup LwIP task */
        xSemaphoreGiveFromISR(*(QueueHandle_t *)(r_xSemaphore.sem), &xHigherPriorityTaskWoken);
    }

    /* clear the enet DMA Rx interrupt pending bits */
    enet_interrupt_flag_clear(ENET0, ENET_DMA_INT_FLAG_RS_CLR);
    enet_interrupt_flag_clear(ENET0, ENET_DMA_INT_FLAG_NI_CLR);

    /* switch tasks if necessary */
    if(pdFALSE != xHigherPriorityTaskWoken) {
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}


