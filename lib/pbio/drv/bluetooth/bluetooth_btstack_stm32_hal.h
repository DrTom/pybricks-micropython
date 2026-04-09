// SPDX-License-Identifier: MIT
// Copyright (c) 2020 The Pybricks Authors

// STM32 HAL UART driver for BlueKitchen BTStack.

#ifndef _INTERNAL_PBDRV_BLUETOOTH_BTSTACK_STM32_HAL_H_
#define _INTERNAL_PBDRV_BLUETOOTH_BTSTACK_STM32_HAL_H_

#include STM32_HAL_H
#include <hci_transport.h>
#include <btstack.h>

#include <pbdrv/gpio.h>

const btstack_control_t *pbdrv_bluetooth_btstack_stm32_hal_control_instance(void);

const hci_transport_t *pbdrv_bluetooth_btstack_stm32_hal_transport_instance(void);

const void *pbdrv_bluetooth_btstack_stm32_hal_transport_config(void);

/** BlueKitchen BTStack driver platform-specific data. */
typedef struct {
    /** GPIO connected to enable pin. */
    pbdrv_gpio_t enable_gpio;
    /** UART connected to the Bluetooth chip. */
    USART_TypeDef *uart;
    /** UART transmit DMA/BDMA instance. */
    void *tx_dma;
    /** UART receive DMA/BDMA instance. */
    void *rx_dma;
    /** UART transmit DMA channel/request selector. */
    uint32_t tx_dma_ch;
    /** UART receive DMA channel/request selector. */
    uint32_t rx_dma_ch;
    /** UART interrupt. */
    IRQn_Type uart_irq;
    /** UART transmit DMA interrupt. */
    IRQn_Type tx_dma_irq;
    /** UART receive DMA interrupt. */
    IRQn_Type rx_dma_irq;
} pbdrv_bluetooth_btstack_stm32_platform_data_t;

// defined in platform.c
extern const pbdrv_bluetooth_btstack_stm32_platform_data_t
    pbdrv_bluetooth_btstack_stm32_platform_data;

void pbdrv_bluetooth_btstack_stm32_hal_handle_tx_dma_irq(void);
void pbdrv_bluetooth_btstack_stm32_hal_handle_rx_dma_irq(void);
void pbdrv_bluetooth_btstack_stm32_hal_handle_uart_irq(void);

// Diagnostic counters for disconnect/re-advertise path (defined in stm32_hal.c)
#if defined(PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY) && PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
extern volatile uint32_t pbdrv_btstack_diag_start_adv_called;
extern volatile uint32_t pbdrv_btstack_diag_start_adv_hci_disabled;
extern volatile uint32_t pbdrv_btstack_diag_start_adv_already_adv;
extern volatile uint32_t pbdrv_btstack_diag_start_adv_busy;
extern volatile uint32_t pbdrv_btstack_diag_start_adv_scheduled;
extern volatile uint32_t pbdrv_btstack_diag_adv_func_entered;
extern volatile uint32_t pbdrv_btstack_diag_adv_func_ble_unsupported;
extern volatile uint32_t pbdrv_btstack_diag_adv_func_no_host_slot;
extern volatile uint32_t pbdrv_btstack_diag_adv_func_gap_enable_called;
extern volatile uint32_t pbdrv_btstack_diag_adv_func_timed_out;
extern volatile uint32_t pbdrv_btstack_diag_adv_func_success;
#endif

#endif // _INTERNAL_PBDRV_BLUETOOTH_BTSTACK_STM32_HAL_H_
