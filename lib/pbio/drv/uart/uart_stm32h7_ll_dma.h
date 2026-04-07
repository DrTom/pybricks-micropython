// SPDX-License-Identifier: MIT

// UART driver for STM32H7x using DMA.

#ifndef _INTERNAL_PBDRV_UART_STM32H7_LL_DMA_H_
#define _INTERNAL_PBDRV_UART_STM32H7_LL_DMA_H_

#include <stdint.h>

#include "pbdrvconfig.h"

#include <stm32h743xx.h>

typedef struct {
    DMA_TypeDef *tx_dma;
    uint32_t tx_dma_stream;
    uint32_t tx_dma_req;
    IRQn_Type tx_dma_irq;
    DMA_TypeDef *rx_dma;
    uint32_t rx_dma_stream;
    uint32_t rx_dma_req;
    IRQn_Type rx_dma_irq;
    USART_TypeDef *uart;
    IRQn_Type uart_irq;
} pbdrv_uart_stm32h7_ll_dma_platform_data_t;

extern const pbdrv_uart_stm32h7_ll_dma_platform_data_t
    pbdrv_uart_stm32h7_ll_dma_platform_data[PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART];

void pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(uint8_t id);
void pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(uint8_t id);
void pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(uint8_t id);

#endif // _INTERNAL_PBDRV_UART_STM32H7_LL_DMA_H_
