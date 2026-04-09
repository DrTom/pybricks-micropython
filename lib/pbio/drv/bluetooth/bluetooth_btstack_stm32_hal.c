// SPDX-License-Identifier: MIT
// Copyright (c) 2020-2025 The Pybricks Authors

// STM32 HAL UART and GPIO driver for BlueKitchen BTStack.

// IMPORTANT: This driver requires a patched STM32 HAL to fix some data loss
// issues. See https://github.com/micropython/stm32lib/pull/12.

#include <pbdrv/config.h>

#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32

#include <btstack.h>
#undef UNUSED // btstack and stm32 both define UNUSED
#include STM32_HAL_H

#include "bluetooth_btstack.h"
#include "bluetooth_btstack_stm32_hal.h"
#include "hci_transport_h4.h"

#include "btstack_chipset_cc256x.h"

#include <pbdrv/gpio.h>

#include <pbio/error.h>

#ifndef PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_INIT
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_INIT 115200
#endif

#ifndef PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_MAIN
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_MAIN 3000000
#endif

#ifndef PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_FLOWCONTROL
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_FLOWCONTROL 1
#endif

#ifndef PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY 0
#endif

static bool btstack_use_dma;

#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
volatile uint32_t pbdrv_btstack_stm32_init_count;
volatile uint32_t pbdrv_btstack_stm32_hci_event_count;
volatile uint32_t pbdrv_btstack_stm32_hci_acl_count;
volatile uint32_t pbdrv_btstack_stm32_hci_cmd_count;
volatile uint32_t pbdrv_btstack_stm32_hci_iso_count;
volatile uint32_t pbdrv_btstack_stm32_hci_unknown_count;
volatile uint32_t pbdrv_btstack_stm32_hci_last_packet_type;
volatile uint32_t pbdrv_btstack_stm32_hci_last_event_code;
volatile uint32_t pbdrv_btstack_stm32_hci_last_opcode;
volatile uint32_t pbdrv_btstack_stm32_hci_last_status;
volatile uint32_t pbdrv_btstack_stm32_hci_state_event_count;
volatile uint32_t pbdrv_btstack_stm32_hci_last_state;
volatile uint32_t pbdrv_btstack_stm32_hci_le_meta_event_count;
volatile uint32_t pbdrv_btstack_stm32_hci_last_le_subevent;
volatile uint32_t pbdrv_btstack_stm32_hci_disconnect_event_count;
volatile uint32_t pbdrv_btstack_stm32_uart_send_block_count;
volatile uint32_t pbdrv_btstack_stm32_uart_send_bytes;
volatile uint32_t pbdrv_btstack_stm32_uart_recv_block_count;
volatile uint32_t pbdrv_btstack_stm32_uart_recv_bytes;
volatile uint32_t pbdrv_btstack_stm32_uart_tx_irq_count;
volatile uint32_t pbdrv_btstack_stm32_uart_rx_irq_count;

typedef struct {
    uint32_t seq;
    uint32_t init_count;
    uint32_t use_dma;
    uint32_t baudrate_init;
    uint32_t baudrate_main;
    uint32_t flowcontrol;
    uint32_t hci_event_count;
    uint32_t hci_acl_count;
    uint32_t hci_cmd_count;
    uint32_t hci_iso_count;
    uint32_t hci_unknown_count;
    uint32_t hci_last_packet_type;
    uint32_t hci_last_event_code;
    uint32_t hci_last_opcode;
    uint32_t hci_last_status;
    uint32_t hci_state_event_count;
    uint32_t hci_last_state;
    uint32_t hci_le_meta_event_count;
    uint32_t hci_last_le_subevent;
    uint32_t hci_disconnect_event_count;
    uint32_t uart_send_block_count;
    uint32_t uart_send_bytes;
    uint32_t uart_recv_block_count;
    uint32_t uart_recv_bytes;
    uint32_t uart_tx_irq_count;
    uint32_t uart_rx_irq_count;
} pbdrv_btstack_stm32_telemetry_snapshot_t;

volatile pbdrv_btstack_stm32_telemetry_snapshot_t pbdrv_btstack_stm32_telemetry_snapshot;

static void pbdrv_btstack_stm32_telemetry_refresh(void) {
    pbdrv_btstack_stm32_telemetry_snapshot.seq++;
    pbdrv_btstack_stm32_telemetry_snapshot.init_count = pbdrv_btstack_stm32_init_count;
    pbdrv_btstack_stm32_telemetry_snapshot.use_dma = btstack_use_dma ? 1 : 0;
    pbdrv_btstack_stm32_telemetry_snapshot.baudrate_init = PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_INIT;
    pbdrv_btstack_stm32_telemetry_snapshot.baudrate_main = PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_MAIN;
    pbdrv_btstack_stm32_telemetry_snapshot.flowcontrol = PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_FLOWCONTROL;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_event_count = pbdrv_btstack_stm32_hci_event_count;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_acl_count = pbdrv_btstack_stm32_hci_acl_count;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_cmd_count = pbdrv_btstack_stm32_hci_cmd_count;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_iso_count = pbdrv_btstack_stm32_hci_iso_count;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_unknown_count = pbdrv_btstack_stm32_hci_unknown_count;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_last_packet_type = pbdrv_btstack_stm32_hci_last_packet_type;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_last_event_code = pbdrv_btstack_stm32_hci_last_event_code;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_last_opcode = pbdrv_btstack_stm32_hci_last_opcode;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_last_status = pbdrv_btstack_stm32_hci_last_status;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_state_event_count = pbdrv_btstack_stm32_hci_state_event_count;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_last_state = pbdrv_btstack_stm32_hci_last_state;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_le_meta_event_count = pbdrv_btstack_stm32_hci_le_meta_event_count;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_last_le_subevent = pbdrv_btstack_stm32_hci_last_le_subevent;
    pbdrv_btstack_stm32_telemetry_snapshot.hci_disconnect_event_count = pbdrv_btstack_stm32_hci_disconnect_event_count;
    pbdrv_btstack_stm32_telemetry_snapshot.uart_send_block_count = pbdrv_btstack_stm32_uart_send_block_count;
    pbdrv_btstack_stm32_telemetry_snapshot.uart_send_bytes = pbdrv_btstack_stm32_uart_send_bytes;
    pbdrv_btstack_stm32_telemetry_snapshot.uart_recv_block_count = pbdrv_btstack_stm32_uart_recv_block_count;
    pbdrv_btstack_stm32_telemetry_snapshot.uart_recv_bytes = pbdrv_btstack_stm32_uart_recv_bytes;
    pbdrv_btstack_stm32_telemetry_snapshot.uart_tx_irq_count = pbdrv_btstack_stm32_uart_tx_irq_count;
    pbdrv_btstack_stm32_telemetry_snapshot.uart_rx_irq_count = pbdrv_btstack_stm32_uart_rx_irq_count;
    pbdrv_btstack_stm32_telemetry_snapshot.seq++;
}
#endif

pbio_error_t pbdrv_bluetooth_btstack_platform_init(void) {
#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    pbdrv_btstack_stm32_init_count++;
    pbdrv_btstack_stm32_hci_event_count = 0;
    pbdrv_btstack_stm32_hci_acl_count = 0;
    pbdrv_btstack_stm32_hci_cmd_count = 0;
    pbdrv_btstack_stm32_hci_iso_count = 0;
    pbdrv_btstack_stm32_hci_unknown_count = 0;
    pbdrv_btstack_stm32_hci_last_packet_type = 0;
    pbdrv_btstack_stm32_hci_last_event_code = 0;
    pbdrv_btstack_stm32_hci_last_opcode = 0;
    pbdrv_btstack_stm32_hci_last_status = 0;
    pbdrv_btstack_stm32_hci_state_event_count = 0;
    pbdrv_btstack_stm32_hci_last_state = 0;
    pbdrv_btstack_stm32_hci_le_meta_event_count = 0;
    pbdrv_btstack_stm32_hci_last_le_subevent = 0;
    pbdrv_btstack_stm32_hci_disconnect_event_count = 0;
    pbdrv_btstack_stm32_uart_send_block_count = 0;
    pbdrv_btstack_stm32_uart_send_bytes = 0;
    pbdrv_btstack_stm32_uart_recv_block_count = 0;
    pbdrv_btstack_stm32_uart_recv_bytes = 0;
    pbdrv_btstack_stm32_uart_tx_irq_count = 0;
    pbdrv_btstack_stm32_uart_rx_irq_count = 0;
    pbdrv_btstack_stm32_telemetry_refresh();
#endif
    return PBIO_SUCCESS;
}

void pbdrv_bluetooth_btstack_platform_poll(void) {
#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    pbdrv_btstack_stm32_telemetry_refresh();
#endif
}

void pbdrv_bluetooth_btstack_platform_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)channel;
#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    pbdrv_btstack_stm32_hci_last_packet_type = packet_type;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            pbdrv_btstack_stm32_hci_event_count++;
            if (size > 0) {
                pbdrv_btstack_stm32_hci_last_event_code = packet[0];

                if (packet[0] == BTSTACK_EVENT_STATE && size >= 3) {
                    pbdrv_btstack_stm32_hci_state_event_count++;
                    pbdrv_btstack_stm32_hci_last_state = packet[2];
                } else if (packet[0] == HCI_EVENT_LE_META && size >= 3) {
                    pbdrv_btstack_stm32_hci_le_meta_event_count++;
                    pbdrv_btstack_stm32_hci_last_le_subevent = packet[2];
                } else if (packet[0] == HCI_EVENT_DISCONNECTION_COMPLETE) {
                    pbdrv_btstack_stm32_hci_disconnect_event_count++;
                }

                if (packet[0] == HCI_EVENT_COMMAND_COMPLETE && size >= 6) {
                    pbdrv_btstack_stm32_hci_last_opcode = packet[3] | (((uint16_t)packet[4]) << 8);
                    pbdrv_btstack_stm32_hci_last_status = packet[5];
                } else if (packet[0] == HCI_EVENT_COMMAND_STATUS && size >= 6) {
                    pbdrv_btstack_stm32_hci_last_opcode = packet[4] | (((uint16_t)packet[5]) << 8);
                    pbdrv_btstack_stm32_hci_last_status = packet[2];
                }
            }
            break;
        case HCI_ACL_DATA_PACKET:
            pbdrv_btstack_stm32_hci_acl_count++;
            break;
        case HCI_COMMAND_DATA_PACKET:
            pbdrv_btstack_stm32_hci_cmd_count++;
            break;
#ifdef HCI_ISO_DATA_PACKET
        case HCI_ISO_DATA_PACKET:
            pbdrv_btstack_stm32_hci_iso_count++;
            break;
#endif
        default:
            pbdrv_btstack_stm32_hci_unknown_count++;
            break;
    }
    pbdrv_btstack_stm32_telemetry_refresh();
#else
    (void)packet_type;
    (void)packet;
    (void)size;
#endif
}

const pbdrv_bluetooth_btstack_chipset_info_t *pbdrv_bluetooth_btstack_set_chipset(pbdrv_bluetooth_btstack_local_version_info_t *device_info) {

    const pbdrv_bluetooth_btstack_platform_data_t *pdata =
        &pbdrv_bluetooth_btstack_platform_data;

#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_CC2564C
    // All platforms supported by this abstraction use a cc2560x with the same
    // init script.
    extern const pbdrv_bluetooth_btstack_chipset_info_t cc2564c_info;
    btstack_chipset_cc256x_set_init_script((uint8_t *)cc2564c_info.init_script, cc2564c_info.init_script_size);

    // Needed to apply init script.
    hci_set_chipset(pdata->chipset_instance());
    return &cc2564c_info;
#else
    static const pbdrv_bluetooth_btstack_chipset_info_t generic_h4_chipset_info = {
        .supports_ble = true,
    };

    if (pdata->chipset_instance) {
        hci_set_chipset(pdata->chipset_instance());
    }

    return &generic_h4_chipset_info;
#endif
};

static int btstack_control_gpio_on(void) {
    const pbdrv_bluetooth_btstack_stm32_platform_data_t *pdata =
        &pbdrv_bluetooth_btstack_stm32_platform_data;

    pbdrv_gpio_out_high(&pdata->enable_gpio);

    return 0;
}

static int btstack_control_gpio_off(void) {
    const pbdrv_bluetooth_btstack_stm32_platform_data_t *pdata =
        &pbdrv_bluetooth_btstack_stm32_platform_data;

    pbdrv_gpio_out_low(&pdata->enable_gpio);

    return 0;
}

static void btstack_control_gpio_init(const void *config) {
    btstack_control_gpio_off();
}

static const btstack_control_t btstack_control_gpio = {
    .init = btstack_control_gpio_init,
    .on = btstack_control_gpio_on,
    .off = btstack_control_gpio_off,
    .sleep = NULL,
    .wake = NULL,
    .register_for_power_notifications = NULL,
};

const btstack_control_t *pbdrv_bluetooth_btstack_stm32_hal_control_instance(void) {
    return &btstack_control_gpio;
}

static UART_HandleTypeDef btstack_huart;
static DMA_HandleTypeDef btstack_rx_hdma;
static DMA_HandleTypeDef btstack_tx_hdma;

// uart config
static const btstack_uart_config_t *uart_config;

// data source for integration with BTstack Runloop
static btstack_data_source_t transport_data_source;

static volatile bool send_complete;
static volatile bool receive_complete;

// callbacks
static void (*block_sent)(void);
static void (*block_received)(void);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    (void)huart;
    pbdrv_btstack_stm32_uart_tx_irq_count++;
#else
    (void)huart;
#endif
    send_complete = true;
    btstack_run_loop_poll_data_sources_from_irq();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    (void)huart;
    pbdrv_btstack_stm32_uart_rx_irq_count++;
#else
    (void)huart;
#endif
    receive_complete = true;
    btstack_run_loop_poll_data_sources_from_irq();
}

static int btstack_stm32_hal_init(const btstack_uart_config_t *config) {
    const pbdrv_bluetooth_btstack_stm32_platform_data_t *pdata =
        &pbdrv_bluetooth_btstack_stm32_platform_data;

    uart_config = config;

    btstack_use_dma = pdata->tx_dma != NULL && pdata->rx_dma != NULL;

#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    pbdrv_btstack_stm32_telemetry_refresh();
#endif

    if (btstack_use_dma) {
        btstack_tx_hdma.Instance = pdata->tx_dma;
        #if defined(STM32H7)
        btstack_tx_hdma.Init.Request = pdata->tx_dma_ch;
        #else
        btstack_tx_hdma.Init.Channel = pdata->tx_dma_ch;
        #endif
        btstack_tx_hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
        btstack_tx_hdma.Init.PeriphInc = DMA_PINC_DISABLE;
        btstack_tx_hdma.Init.MemInc = DMA_MINC_ENABLE;
        btstack_tx_hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        btstack_tx_hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        btstack_tx_hdma.Init.Mode = DMA_NORMAL;
        btstack_tx_hdma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        btstack_tx_hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        btstack_tx_hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
        btstack_tx_hdma.Init.MemBurst = DMA_MBURST_SINGLE;
        btstack_tx_hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
        HAL_DMA_Init(&btstack_tx_hdma);

        btstack_rx_hdma.Instance = pdata->rx_dma;
        #if defined(STM32H7)
        btstack_rx_hdma.Init.Request = pdata->rx_dma_ch;
        #else
        btstack_rx_hdma.Init.Channel = pdata->rx_dma_ch;
        #endif
        btstack_rx_hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
        btstack_rx_hdma.Init.PeriphInc = DMA_PINC_DISABLE;
        btstack_rx_hdma.Init.MemInc = DMA_MINC_ENABLE;
        btstack_rx_hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        btstack_rx_hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        btstack_rx_hdma.Init.Mode = DMA_NORMAL;
        btstack_rx_hdma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        btstack_rx_hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        btstack_rx_hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
        btstack_rx_hdma.Init.MemBurst = DMA_MBURST_SINGLE;
        btstack_rx_hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
        HAL_DMA_Init(&btstack_rx_hdma);
    }

    btstack_huart.Instance = pdata->uart;
    btstack_huart.Init.BaudRate = config->baudrate;
    btstack_huart.Init.WordLength = UART_WORDLENGTH_8B;
    btstack_huart.Init.StopBits = UART_STOPBITS_1;
    btstack_huart.Init.Parity = UART_PARITY_NONE;
    btstack_huart.Init.Mode = UART_MODE_TX_RX;
    btstack_huart.Init.HwFlowCtl = config->flowcontrol ? UART_HWCONTROL_RTS_CTS : UART_HWCONTROL_NONE;
    btstack_huart.Init.OverSampling = UART_OVERSAMPLING_16;
    #if defined(STM32H7)
    btstack_huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    btstack_huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    btstack_huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    btstack_huart.FifoMode = UART_FIFOMODE_DISABLE;
    #endif
    HAL_UART_Init(&btstack_huart);
    #if defined(STM32H7)
    HAL_UARTEx_DisableFifoMode(&btstack_huart);
    #endif

    if (btstack_use_dma) {
        __HAL_LINKDMA(&btstack_huart, hdmatx, btstack_tx_hdma);
        __HAL_LINKDMA(&btstack_huart, hdmarx, btstack_rx_hdma);
    }

    if (btstack_use_dma) {
        HAL_NVIC_SetPriority(pdata->tx_dma_irq, 1, 2);
        HAL_NVIC_EnableIRQ(pdata->tx_dma_irq);
        HAL_NVIC_SetPriority(pdata->rx_dma_irq, 1, 1);
        HAL_NVIC_EnableIRQ(pdata->rx_dma_irq);
    }
    HAL_NVIC_SetPriority(pdata->uart_irq, 1, 0);
    HAL_NVIC_EnableIRQ(pdata->uart_irq);

    return 0;
}

static void btstack_stm32_hal_process(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type) {
    switch (callback_type) {
        case DATA_SOURCE_CALLBACK_POLL:
            if (send_complete) {
                send_complete = false;
                if (block_sent) {
                    block_sent();
                }
            }
            if (receive_complete) {
                receive_complete = false;
                if (block_received) {
                    block_received();
                }
            }
            break;
        default:
            break;
    }
}

static int btstack_stm32_hal_set_baudrate(uint32_t baud) {
    btstack_huart.Init.BaudRate = baud;
    return HAL_UART_Init(&btstack_huart) == HAL_OK ? 0 : -1;
}

static int btstack_stm32_hal_open(void) {
    btstack_stm32_hal_set_baudrate(uart_config->baudrate);

    // set up polling data_source
    btstack_run_loop_set_data_source_handler(&transport_data_source, &btstack_stm32_hal_process);
    btstack_run_loop_enable_data_source_callbacks(&transport_data_source, DATA_SOURCE_CALLBACK_POLL);
    btstack_run_loop_add_data_source(&transport_data_source);

    return 0;
}

static int btstack_stm32_hal_close(void) {
    // remove data source
    btstack_run_loop_disable_data_source_callbacks(&transport_data_source, DATA_SOURCE_CALLBACK_POLL);
    btstack_run_loop_remove_data_source(&transport_data_source);

    return 0;
}

static void btstack_stm32_hal_set_block_received(void (*handler)(void)) {
    block_received = handler;
}

static void btstack_stm32_hal_set_block_sent(void (*handler)(void)) {
    block_sent = handler;
}

static int btstack_stm32_hal_set_parity(int parity) {
    return 0;
}

static void btstack_stm32_hal_receive_block(uint8_t *buffer, uint16_t len) {
#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    pbdrv_btstack_stm32_uart_recv_block_count++;
    pbdrv_btstack_stm32_uart_recv_bytes += len;
#endif
    HAL_StatusTypeDef hal_status;

    if (btstack_use_dma) {
        hal_status = HAL_UART_Receive_DMA(&btstack_huart, buffer, len);
        if (hal_status != HAL_OK) {
            HAL_UART_AbortReceive(&btstack_huart);
            (void)HAL_UART_Receive_DMA(&btstack_huart, buffer, len);
        }
    } else {
        hal_status = HAL_UART_Receive_IT(&btstack_huart, buffer, len);
        if (hal_status != HAL_OK) {
            HAL_UART_AbortReceive(&btstack_huart);
            (void)HAL_UART_Receive_IT(&btstack_huart, buffer, len);
        }
    }
}

static void btstack_stm32_hal_send_block(const uint8_t *data, uint16_t size) {
#if PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY
    pbdrv_btstack_stm32_uart_send_block_count++;
    pbdrv_btstack_stm32_uart_send_bytes += size;
#endif
    HAL_StatusTypeDef hal_status;

    if (btstack_use_dma) {
        hal_status = HAL_UART_Transmit_DMA(&btstack_huart, (uint8_t *)data, size);
        if (hal_status != HAL_OK) {
            HAL_UART_AbortTransmit(&btstack_huart);
            (void)HAL_UART_Transmit_DMA(&btstack_huart, (uint8_t *)data, size);
        }
    } else {
        hal_status = HAL_UART_Transmit_IT(&btstack_huart, (uint8_t *)data, size);
        if (hal_status != HAL_OK) {
            HAL_UART_AbortTransmit(&btstack_huart);
            (void)HAL_UART_Transmit_IT(&btstack_huart, (uint8_t *)data, size);
        }
    }
}

static const btstack_uart_block_t btstack_stm32_hal = {
    .init = btstack_stm32_hal_init,
    .open = btstack_stm32_hal_open,
    .close = btstack_stm32_hal_close,
    .set_block_received = btstack_stm32_hal_set_block_received,
    .set_block_sent = btstack_stm32_hal_set_block_sent,
    .set_baudrate = btstack_stm32_hal_set_baudrate,
    .set_parity = btstack_stm32_hal_set_parity,
    .set_flowcontrol = NULL,
    .receive_block = btstack_stm32_hal_receive_block,
    .send_block = btstack_stm32_hal_send_block,
    .get_supported_sleep_modes = NULL,
    .set_sleep = NULL,
    .set_wakeup_handler = NULL,
};

const hci_transport_t *pbdrv_bluetooth_btstack_stm32_hal_transport_instance(void) {
    return hci_transport_h4_instance_for_uart(&btstack_stm32_hal);
}

const void *pbdrv_bluetooth_btstack_stm32_hal_transport_config(void) {
    static const hci_transport_config_uart_t config = {
        .type = HCI_TRANSPORT_CONFIG_UART,
        .baudrate_init = PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_INIT,
        .baudrate_main = PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_MAIN,
        .flowcontrol = PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_FLOWCONTROL,
        .device_name = NULL,
    };
    return &config;
}

void pbdrv_bluetooth_btstack_stm32_hal_handle_tx_dma_irq(void) {
    if (btstack_use_dma) {
        HAL_DMA_IRQHandler(&btstack_tx_hdma);
    }
}

void pbdrv_bluetooth_btstack_stm32_hal_handle_rx_dma_irq(void) {
    if (btstack_use_dma) {
        HAL_DMA_IRQHandler(&btstack_rx_hdma);
    }
}

void pbdrv_bluetooth_btstack_stm32_hal_handle_uart_irq(void) {
    HAL_UART_IRQHandler(&btstack_huart);
}

#endif // PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32
