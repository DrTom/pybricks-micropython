// SPDX-License-Identifier: MIT

// UART driver for STM32H7x using IRQ.

#include <pbdrv/config.h>

#if PBDRV_CONFIG_UART_STM32H7_LL_IRQ

#include <stdbool.h>
#include <stdint.h>

#include <pbdrv/uart.h>

#include <pbio/error.h>
#include <pbio/os.h>

#include <lwrb/lwrb.h>

#include "./uart_stm32h7_ll_irq.h"

#define RX_DATA_SIZE 64

#define UART_CR1_RXNEIE USART_CR1_RXNEIE_RXFNEIE
#define UART_CR1_TXEIE  USART_CR1_TXEIE_TXFNFIE
#define UART_ISR_RXNE   USART_ISR_RXNE_RXFNE
#define UART_ISR_TXE    USART_ISR_TXE_TXFNF

struct _pbdrv_uart_dev_t {
    const pbdrv_uart_stm32h7_ll_irq_platform_data_t *pdata;
    lwrb_t rx_buf;
    pbio_os_timer_t read_timer;
    pbio_os_timer_t write_timer;
    uint8_t *read_buf;
    uint32_t read_length;
    uint32_t read_pos;
    const uint8_t *write_buf;
    uint32_t write_length;
    volatile uint32_t write_pos;
};

static pbdrv_uart_dev_t uart_devs[PBDRV_CONFIG_UART_STM32H7_LL_IRQ_NUM_UART];
static uint8_t pbdrv_uart_rx_data[PBDRV_CONFIG_UART_STM32H7_LL_IRQ_NUM_UART][RX_DATA_SIZE];

static uint32_t pbdrv_uart_get_peripheral_clock(USART_TypeDef *uart) {
    (void)uart;
    // Initial bring-up keeps APB clocks at SYSCLK (64 MHz).
    return PBDRV_CONFIG_SYS_CLOCK_RATE;
}

pbio_error_t pbdrv_uart_get_instance(uint8_t id, pbdrv_uart_dev_t **uart_dev) {
    if (id >= PBDRV_CONFIG_UART_STM32H7_LL_IRQ_NUM_UART) {
        return PBIO_ERROR_INVALID_ARG;
    }
    pbdrv_uart_dev_t *dev = &uart_devs[id];
    if (!dev->pdata) {
        return PBIO_ERROR_AGAIN;
    }
    *uart_dev = dev;
    return PBIO_SUCCESS;
}

uint32_t pbdrv_uart_in_waiting(pbdrv_uart_dev_t *uart_dev) {
    return lwrb_get_full(&uart_dev->rx_buf);
}

pbio_error_t pbdrv_uart_read(pbio_os_state_t *state, pbdrv_uart_dev_t *uart, uint8_t *msg, uint32_t length, uint32_t timeout) {

    PBIO_OS_ASYNC_BEGIN(state);

    if (uart->read_buf) {
        return PBIO_ERROR_BUSY;
    }

    uart->read_buf = msg;
    uart->read_length = length;
    uart->read_pos = 0;

    if (timeout) {
        pbio_os_timer_set(&uart->read_timer, timeout);
    }

    PBIO_OS_AWAIT_UNTIL(state, ({
        uart->read_pos += lwrb_read(&uart->rx_buf, &uart->read_buf[uart->read_pos], uart->read_length - uart->read_pos);
        uart->read_pos == uart->read_length || (timeout && pbio_os_timer_is_expired(&uart->read_timer));
    }));

    uart->read_buf = NULL;

    if (timeout && pbio_os_timer_is_expired(&uart->read_timer)) {
        return PBIO_ERROR_TIMEDOUT;
    }

    PBIO_OS_ASYNC_END(PBIO_SUCCESS);
}

pbio_error_t pbdrv_uart_write(pbio_os_state_t *state, pbdrv_uart_dev_t *uart, const uint8_t *msg, uint32_t length, uint32_t timeout) {

    PBIO_OS_ASYNC_BEGIN(state);

    if (uart->write_buf) {
        return PBIO_ERROR_BUSY;
    }

    uart->write_buf = msg;
    uart->write_length = length;
    uart->write_pos = 0;

    if (timeout) {
        pbio_os_timer_set(&uart->write_timer, timeout);
    }

    uart->pdata->uart->CR1 |= UART_CR1_TXEIE;

    PBIO_OS_AWAIT_UNTIL(state, uart->write_pos == uart->write_length || (timeout && pbio_os_timer_is_expired(&uart->write_timer)));

    uart->write_buf = NULL;

    if (timeout && pbio_os_timer_is_expired(&uart->write_timer)) {
        uart->pdata->uart->CR1 &= ~(UART_CR1_TXEIE | USART_CR1_TCIE);
        return PBIO_ERROR_TIMEDOUT;
    }

    PBIO_OS_ASYNC_END(PBIO_SUCCESS);
}

void pbdrv_uart_set_baud_rate(pbdrv_uart_dev_t *uart, uint32_t baud) {
    if (!baud) {
        return;
    }

    USART_TypeDef *USARTx = uart->pdata->uart;
    uint32_t periphclk = pbdrv_uart_get_peripheral_clock(USARTx);

    USARTx->BRR = (periphclk + (baud / 2)) / baud;
}

void pbdrv_uart_flush(pbdrv_uart_dev_t *uart) {
    uart->write_buf = NULL;
    uart->write_length = 0;
    uart->write_pos = 0;
    uart->read_buf = NULL;
    uart->read_length = 0;
    uart->read_pos = 0;
    lwrb_reset(&uart->rx_buf);
}

void pbdrv_uart_stm32h7_ll_irq_handle_irq(uint8_t id) {
    pbdrv_uart_dev_t *uart = &uart_devs[id];
    USART_TypeDef *USARTx = uart->pdata->uart;
    uint32_t isr = USARTx->ISR;

    if (isr & UART_ISR_RXNE) {
        uint8_t c = (uint8_t)USARTx->RDR;
        lwrb_write(&uart->rx_buf, &c, 1);
        pbio_os_request_poll();
    }

    if (isr & USART_ISR_ORE) {
        USARTx->ICR = USART_ICR_ORECF;
    }

    if ((USARTx->CR1 & UART_CR1_TXEIE) && (isr & UART_ISR_TXE) && uart->write_buf) {
        USARTx->TDR = uart->write_buf[uart->write_pos++];
        if (uart->write_pos == uart->write_length) {
            USARTx->CR1 &= ~UART_CR1_TXEIE;
            USARTx->CR1 |= USART_CR1_TCIE;
        }
    }

    if ((USARTx->CR1 & USART_CR1_TCIE) && (isr & USART_ISR_TC)) {
        USARTx->CR1 &= ~USART_CR1_TCIE;
        USARTx->ICR = USART_ICR_TCCF;
        pbio_os_request_poll();
    }
}

void pbdrv_uart_stop(pbdrv_uart_dev_t *uart) {
    uart->pdata->uart->CR1 = 0;
    NVIC_DisableIRQ(uart->pdata->irq);
}

void pbdrv_uart_init(void) {
    for (int i = 0; i < PBDRV_CONFIG_UART_STM32H7_LL_IRQ_NUM_UART; i++) {
        const pbdrv_uart_stm32h7_ll_irq_platform_data_t *pdata = &pbdrv_uart_stm32h7_ll_irq_platform_data[i];
        pbdrv_uart_dev_t *uart = &uart_devs[i];

        uart->pdata = pdata;
        lwrb_init(&uart->rx_buf, pbdrv_uart_rx_data[i], RX_DATA_SIZE);

        pdata->uart->CR1 = 0;
        pdata->uart->CR2 = 0;
        pdata->uart->CR3 = 0;

        pbdrv_uart_set_baud_rate(uart, 115200);

        pdata->uart->CR1 = USART_CR1_TE | USART_CR1_RE | UART_CR1_RXNEIE | USART_CR1_UE;

        NVIC_SetPriority(pdata->irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
        NVIC_EnableIRQ(pdata->irq);
    }
}

#endif // PBDRV_CONFIG_UART_STM32H7_LL_IRQ
