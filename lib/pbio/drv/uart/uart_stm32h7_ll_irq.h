// SPDX-License-Identifier: MIT

#ifndef _INTERNAL_PBDRV_UART_STM32H7_LL_IRQ_H_
#define _INTERNAL_PBDRV_UART_STM32H7_LL_IRQ_H_

#include <stdint.h>

#include "pbdrvconfig.h"

#include <stm32h743xx.h>

typedef struct {
    USART_TypeDef *uart;
    IRQn_Type irq;
} pbdrv_uart_stm32h7_ll_irq_platform_data_t;

extern const pbdrv_uart_stm32h7_ll_irq_platform_data_t
    pbdrv_uart_stm32h7_ll_irq_platform_data[PBDRV_CONFIG_UART_STM32H7_LL_IRQ_NUM_UART];

void pbdrv_uart_stm32h7_ll_irq_handle_irq(uint8_t id);

#endif // _INTERNAL_PBDRV_UART_STM32H7_LL_IRQ_H_
