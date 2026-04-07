// SPDX-License-Identifier: MIT

// UART driver for STM32H7x using DMA.

#include <pbdrv/config.h>

#if PBDRV_CONFIG_UART_STM32H7_LL_DMA

#include <stdbool.h>
#include <stdint.h>

#include <pbdrv/uart.h>

#include <pbio/error.h>
#include <pbio/os.h>

#include "./uart_stm32h7_ll_dma.h"

#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_usart.h"

#define RX_DATA_SIZE 64

struct _pbdrv_uart_dev_t {
    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata;
    pbio_os_timer_t rx_timer;
    pbio_os_timer_t tx_timer;
    volatile uint8_t *rx_data;
    uint32_t rx_tail;
    uint8_t *read_buf;
    uint32_t read_length;
};

static pbdrv_uart_dev_t uart_devs[PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART];
static volatile uint8_t pbdrv_uart_rx_data[PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART][RX_DATA_SIZE];

static void volatile_copy(volatile uint8_t *src, uint8_t *dst, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}

static void dma_clear_tc(DMA_TypeDef *DMAx, uint32_t stream) {
    switch (stream) {
        case LL_DMA_STREAM_0: LL_DMA_ClearFlag_TC0(DMAx); break;
        case LL_DMA_STREAM_1: LL_DMA_ClearFlag_TC1(DMAx); break;
        case LL_DMA_STREAM_2: LL_DMA_ClearFlag_TC2(DMAx); break;
        case LL_DMA_STREAM_3: LL_DMA_ClearFlag_TC3(DMAx); break;
        case LL_DMA_STREAM_4: LL_DMA_ClearFlag_TC4(DMAx); break;
        case LL_DMA_STREAM_5: LL_DMA_ClearFlag_TC5(DMAx); break;
        case LL_DMA_STREAM_6: LL_DMA_ClearFlag_TC6(DMAx); break;
        case LL_DMA_STREAM_7: LL_DMA_ClearFlag_TC7(DMAx); break;
    }
}

static void dma_clear_ht(DMA_TypeDef *DMAx, uint32_t stream) {
    switch (stream) {
        case LL_DMA_STREAM_0: LL_DMA_ClearFlag_HT0(DMAx); break;
        case LL_DMA_STREAM_1: LL_DMA_ClearFlag_HT1(DMAx); break;
        case LL_DMA_STREAM_2: LL_DMA_ClearFlag_HT2(DMAx); break;
        case LL_DMA_STREAM_3: LL_DMA_ClearFlag_HT3(DMAx); break;
        case LL_DMA_STREAM_4: LL_DMA_ClearFlag_HT4(DMAx); break;
        case LL_DMA_STREAM_5: LL_DMA_ClearFlag_HT5(DMAx); break;
        case LL_DMA_STREAM_6: LL_DMA_ClearFlag_HT6(DMAx); break;
        case LL_DMA_STREAM_7: LL_DMA_ClearFlag_HT7(DMAx); break;
    }
}

static void dma_clear_te(DMA_TypeDef *DMAx, uint32_t stream) {
    switch (stream) {
        case LL_DMA_STREAM_0: LL_DMA_ClearFlag_TE0(DMAx); break;
        case LL_DMA_STREAM_1: LL_DMA_ClearFlag_TE1(DMAx); break;
        case LL_DMA_STREAM_2: LL_DMA_ClearFlag_TE2(DMAx); break;
        case LL_DMA_STREAM_3: LL_DMA_ClearFlag_TE3(DMAx); break;
        case LL_DMA_STREAM_4: LL_DMA_ClearFlag_TE4(DMAx); break;
        case LL_DMA_STREAM_5: LL_DMA_ClearFlag_TE5(DMAx); break;
        case LL_DMA_STREAM_6: LL_DMA_ClearFlag_TE6(DMAx); break;
        case LL_DMA_STREAM_7: LL_DMA_ClearFlag_TE7(DMAx); break;
    }
}

static bool dma_is_tc(DMA_TypeDef *DMAx, uint32_t stream) {
    switch (stream) {
        case LL_DMA_STREAM_0: return LL_DMA_IsActiveFlag_TC0(DMAx);
        case LL_DMA_STREAM_1: return LL_DMA_IsActiveFlag_TC1(DMAx);
        case LL_DMA_STREAM_2: return LL_DMA_IsActiveFlag_TC2(DMAx);
        case LL_DMA_STREAM_3: return LL_DMA_IsActiveFlag_TC3(DMAx);
        case LL_DMA_STREAM_4: return LL_DMA_IsActiveFlag_TC4(DMAx);
        case LL_DMA_STREAM_5: return LL_DMA_IsActiveFlag_TC5(DMAx);
        case LL_DMA_STREAM_6: return LL_DMA_IsActiveFlag_TC6(DMAx);
        case LL_DMA_STREAM_7: return LL_DMA_IsActiveFlag_TC7(DMAx);
        default: return false;
    }
}

static bool dma_is_ht(DMA_TypeDef *DMAx, uint32_t stream) {
    switch (stream) {
        case LL_DMA_STREAM_0: return LL_DMA_IsActiveFlag_HT0(DMAx);
        case LL_DMA_STREAM_1: return LL_DMA_IsActiveFlag_HT1(DMAx);
        case LL_DMA_STREAM_2: return LL_DMA_IsActiveFlag_HT2(DMAx);
        case LL_DMA_STREAM_3: return LL_DMA_IsActiveFlag_HT3(DMAx);
        case LL_DMA_STREAM_4: return LL_DMA_IsActiveFlag_HT4(DMAx);
        case LL_DMA_STREAM_5: return LL_DMA_IsActiveFlag_HT5(DMAx);
        case LL_DMA_STREAM_6: return LL_DMA_IsActiveFlag_HT6(DMAx);
        case LL_DMA_STREAM_7: return LL_DMA_IsActiveFlag_HT7(DMAx);
        default: return false;
    }
}

pbio_error_t pbdrv_uart_get_instance(uint8_t id, pbdrv_uart_dev_t **uart_dev) {
    if (id >= PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART) {
        return PBIO_ERROR_INVALID_ARG;
    }
    pbdrv_uart_dev_t *dev = &uart_devs[id];
    if (!dev->pdata) {
        return PBIO_ERROR_AGAIN;
    }
    *uart_dev = dev;
    return PBIO_SUCCESS;
}

uint32_t pbdrv_uart_in_waiting(pbdrv_uart_dev_t *uart) {
    uint32_t rx_head = RX_DATA_SIZE - LL_DMA_GetDataLength(uart->pdata->rx_dma, uart->pdata->rx_dma_stream);
    return (rx_head - uart->rx_tail) & (RX_DATA_SIZE - 1);
}

pbio_error_t pbdrv_uart_read(pbio_os_state_t *state, pbdrv_uart_dev_t *uart, uint8_t *msg, uint32_t length, uint32_t timeout) {
    PBIO_OS_ASYNC_BEGIN(state);

    if (uart->read_buf) {
        return PBIO_ERROR_BUSY;
    }

    uart->read_buf = msg;
    uart->read_length = length;

    if (timeout) {
        pbio_os_timer_set(&uart->rx_timer, timeout);
    }

    PBIO_OS_AWAIT_UNTIL(state, pbdrv_uart_in_waiting(uart) >= uart->read_length || (timeout && pbio_os_timer_is_expired(&uart->rx_timer)));
    if (timeout && pbio_os_timer_is_expired(&uart->rx_timer)) {
        uart->read_buf = NULL;
        uart->read_length = 0;
        return PBIO_ERROR_TIMEDOUT;
    }

    if (uart->rx_tail + uart->read_length > RX_DATA_SIZE) {
        uint32_t partial_size = RX_DATA_SIZE - uart->rx_tail;
        volatile_copy(&uart->rx_data[uart->rx_tail], &uart->read_buf[0], partial_size);
        volatile_copy(&uart->rx_data[0], &uart->read_buf[partial_size], uart->read_length - partial_size);
    } else {
        volatile_copy(&uart->rx_data[uart->rx_tail], &uart->read_buf[0], uart->read_length);
    }

    uart->rx_tail = (uart->rx_tail + uart->read_length) & (RX_DATA_SIZE - 1);
    uart->read_buf = NULL;
    uart->read_length = 0;

    PBIO_OS_ASYNC_END(PBIO_SUCCESS);
}

pbio_error_t pbdrv_uart_write(pbio_os_state_t *state, pbdrv_uart_dev_t *uart, const uint8_t *msg, uint32_t length, uint32_t timeout) {
    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = uart->pdata;

    PBIO_OS_ASYNC_BEGIN(state);

    if (LL_USART_IsEnabledDMAReq_TX(pdata->uart)) {
        return PBIO_ERROR_BUSY;
    }

    LL_DMA_DisableStream(pdata->tx_dma, pdata->tx_dma_stream);
    LL_DMA_SetMemoryAddress(pdata->tx_dma, pdata->tx_dma_stream, (uint32_t)msg);
    LL_DMA_SetDataLength(pdata->tx_dma, pdata->tx_dma_stream, length);
    dma_clear_tc(pdata->tx_dma, pdata->tx_dma_stream);
    dma_clear_ht(pdata->tx_dma, pdata->tx_dma_stream);
    dma_clear_te(pdata->tx_dma, pdata->tx_dma_stream);
    LL_DMA_EnableStream(pdata->tx_dma, pdata->tx_dma_stream);
    LL_USART_ClearFlag_TC(pdata->uart);
    LL_USART_EnableDMAReq_TX(pdata->uart);

    if (timeout) {
        pbio_os_timer_set(&uart->tx_timer, timeout);
    }

    PBIO_OS_AWAIT_WHILE(state, LL_USART_IsEnabledDMAReq_TX(pdata->uart) && !(timeout && pbio_os_timer_is_expired(&uart->tx_timer)));
    if ((timeout && pbio_os_timer_is_expired(&uart->tx_timer))) {
        LL_USART_DisableDMAReq_TX(pdata->uart);
        return PBIO_ERROR_TIMEDOUT;
    }

    PBIO_OS_ASYNC_END(PBIO_SUCCESS);
}

void pbdrv_uart_set_baud_rate(pbdrv_uart_dev_t *uart, uint32_t baud) {
    if (!baud) {
        return;
    }
    LL_USART_SetBaudRate(uart->pdata->uart, PBDRV_CONFIG_SYS_CLOCK_RATE, LL_USART_PRESCALER_DIV1, LL_USART_OVERSAMPLING_16, baud);
}

void pbdrv_uart_flush(pbdrv_uart_dev_t *uart) {
    uart->read_buf = NULL;
    uart->read_length = 0;
    uart->rx_tail = RX_DATA_SIZE - LL_DMA_GetDataLength(uart->pdata->rx_dma, uart->pdata->rx_dma_stream);
}

void pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(uint8_t id) {
    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = &pbdrv_uart_stm32h7_ll_dma_platform_data[id];
    if (LL_DMA_IsEnabledIT_TC(pdata->tx_dma, pdata->tx_dma_stream) && dma_is_tc(pdata->tx_dma, pdata->tx_dma_stream)) {
        dma_clear_tc(pdata->tx_dma, pdata->tx_dma_stream);
        LL_USART_DisableDMAReq_TX(pdata->uart);
        pbio_os_request_poll();
    }
}

void pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(uint8_t id) {
    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = &pbdrv_uart_stm32h7_ll_dma_platform_data[id];

    if (LL_DMA_IsEnabledIT_HT(pdata->rx_dma, pdata->rx_dma_stream) && dma_is_ht(pdata->rx_dma, pdata->rx_dma_stream)) {
        dma_clear_ht(pdata->rx_dma, pdata->rx_dma_stream);
        pbio_os_request_poll();
    }

    if (LL_DMA_IsEnabledIT_TC(pdata->rx_dma, pdata->rx_dma_stream) && dma_is_tc(pdata->rx_dma, pdata->rx_dma_stream)) {
        dma_clear_tc(pdata->rx_dma, pdata->rx_dma_stream);
        pbio_os_request_poll();
    }
}

void pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(uint8_t id) {
    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = &pbdrv_uart_stm32h7_ll_dma_platform_data[id];

    if (LL_USART_IsEnabledIT_TC(pdata->uart) && LL_USART_IsActiveFlag_TC(pdata->uart)) {
        LL_USART_DisableIT_TC(pdata->uart);
        LL_USART_ClearFlag_TC(pdata->uart);
        pbio_os_request_poll();
    }

    if (LL_USART_IsEnabledIT_IDLE(pdata->uart) && LL_USART_IsActiveFlag_IDLE(pdata->uart)) {
        LL_USART_ClearFlag_IDLE(pdata->uart);
        pbio_os_request_poll();
    }
}

void pbdrv_uart_stop(pbdrv_uart_dev_t *uart) {
    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = uart->pdata;
    LL_USART_Disable(pdata->uart);
    LL_DMA_DisableStream(pdata->rx_dma, pdata->rx_dma_stream);
    LL_DMA_DisableStream(pdata->tx_dma, pdata->tx_dma_stream);
    NVIC_DisableIRQ(pdata->uart_irq);
    NVIC_DisableIRQ(pdata->rx_dma_irq);
    NVIC_DisableIRQ(pdata->tx_dma_irq);
}

void pbdrv_uart_init(void) {
    for (int i = 0; i < PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART; i++) {
        const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = &pbdrv_uart_stm32h7_ll_dma_platform_data[i];
        volatile uint8_t *rx_data = pbdrv_uart_rx_data[i];
        pbdrv_uart_dev_t *uart = &uart_devs[i];

        uart->pdata = pdata;
        uart->rx_data = rx_data;

        LL_DMA_SetPeriphRequest(pdata->tx_dma, pdata->tx_dma_stream, pdata->tx_dma_req);
        LL_DMA_SetDataTransferDirection(pdata->tx_dma, pdata->tx_dma_stream, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetStreamPriorityLevel(pdata->tx_dma, pdata->tx_dma_stream, LL_DMA_PRIORITY_LOW);
        LL_DMA_SetMode(pdata->tx_dma, pdata->tx_dma_stream, LL_DMA_MODE_NORMAL);
        LL_DMA_SetPeriphIncMode(pdata->tx_dma, pdata->tx_dma_stream, LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(pdata->tx_dma, pdata->tx_dma_stream, LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(pdata->tx_dma, pdata->tx_dma_stream, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(pdata->tx_dma, pdata->tx_dma_stream, LL_DMA_MDATAALIGN_BYTE);
        LL_DMA_SetPeriphAddress(pdata->tx_dma, pdata->tx_dma_stream, (uint32_t)&pdata->uart->TDR);
        LL_DMA_EnableIT_TC(pdata->tx_dma, pdata->tx_dma_stream);

        NVIC_SetPriority(pdata->tx_dma_irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 1));
        NVIC_EnableIRQ(pdata->tx_dma_irq);

        LL_DMA_SetPeriphRequest(pdata->rx_dma, pdata->rx_dma_stream, pdata->rx_dma_req);
        LL_DMA_SetDataTransferDirection(pdata->rx_dma, pdata->rx_dma_stream, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetStreamPriorityLevel(pdata->rx_dma, pdata->rx_dma_stream, LL_DMA_PRIORITY_HIGH);
        LL_DMA_SetMode(pdata->rx_dma, pdata->rx_dma_stream, LL_DMA_MODE_CIRCULAR);
        LL_DMA_SetPeriphIncMode(pdata->rx_dma, pdata->rx_dma_stream, LL_DMA_PERIPH_NOINCREMENT);
        LL_DMA_SetMemoryIncMode(pdata->rx_dma, pdata->rx_dma_stream, LL_DMA_MEMORY_INCREMENT);
        LL_DMA_SetPeriphSize(pdata->rx_dma, pdata->rx_dma_stream, LL_DMA_PDATAALIGN_BYTE);
        LL_DMA_SetMemorySize(pdata->rx_dma, pdata->rx_dma_stream, LL_DMA_MDATAALIGN_BYTE);
        LL_DMA_SetPeriphAddress(pdata->rx_dma, pdata->rx_dma_stream, (uint32_t)&pdata->uart->RDR);
        LL_DMA_SetMemoryAddress(pdata->rx_dma, pdata->rx_dma_stream, (uint32_t)rx_data);
        LL_DMA_SetDataLength(pdata->rx_dma, pdata->rx_dma_stream, RX_DATA_SIZE);
        LL_DMA_EnableIT_HT(pdata->rx_dma, pdata->rx_dma_stream);
        LL_DMA_EnableIT_TC(pdata->rx_dma, pdata->rx_dma_stream);

        NVIC_SetPriority(pdata->rx_dma_irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 1));
        NVIC_EnableIRQ(pdata->rx_dma_irq);

        pbdrv_uart_set_baud_rate(uart, 115200);

        // Enable UART transmitter and receiver before enabling DMA requests.
        pdata->uart->CR1 |= USART_CR1_TE | USART_CR1_RE;

        LL_USART_EnableIT_IDLE(pdata->uart);
        LL_USART_EnableDMAReq_RX(pdata->uart);
        LL_USART_Enable(pdata->uart);

        LL_DMA_EnableStream(pdata->rx_dma, pdata->rx_dma_stream);

        NVIC_SetPriority(pdata->uart_irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
        NVIC_EnableIRQ(pdata->uart_irq);
    }
}

// Bring-up helper for debugger-driven DMA validation.
__attribute__((used)) void pbdrv_uart_stm32h7_ll_dma_test_tx(uint8_t id, const uint8_t *msg, uint32_t length) {
    if (id >= PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART || !msg || !length) {
        return;
    }

    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = &pbdrv_uart_stm32h7_ll_dma_platform_data[id];

    LL_DMA_DisableStream(pdata->tx_dma, pdata->tx_dma_stream);
    LL_DMA_SetMemoryAddress(pdata->tx_dma, pdata->tx_dma_stream, (uint32_t)msg);
    LL_DMA_SetDataLength(pdata->tx_dma, pdata->tx_dma_stream, length);
    dma_clear_tc(pdata->tx_dma, pdata->tx_dma_stream);
    dma_clear_ht(pdata->tx_dma, pdata->tx_dma_stream);
    dma_clear_te(pdata->tx_dma, pdata->tx_dma_stream);
    LL_DMA_EnableStream(pdata->tx_dma, pdata->tx_dma_stream);
    LL_USART_ClearFlag_TC(pdata->uart);
    LL_USART_EnableDMAReq_TX(pdata->uart);
}

// Bring-up helper for debugger-driven DMA validation.
__attribute__((used)) uint32_t pbdrv_uart_stm32h7_ll_dma_test_rx_head(uint8_t id) {
    if (id >= PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART) {
        return 0;
    }
    const pbdrv_uart_stm32h7_ll_dma_platform_data_t *pdata = &pbdrv_uart_stm32h7_ll_dma_platform_data[id];
    return RX_DATA_SIZE - LL_DMA_GetDataLength(pdata->rx_dma, pdata->rx_dma_stream);
}

#endif // PBDRV_CONFIG_UART_STM32H7_LL_DMA
