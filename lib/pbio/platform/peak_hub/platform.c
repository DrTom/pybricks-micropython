// SPDX-License-Identifier: MIT

#include <pbdrv/config.h>

#include <pbdrv/uart.h>

#if PBDRV_CONFIG_USB_STM32H7
#include <pbdrv/usb.h>
#include <lego/usb.h>
#include STM32_HAL_H
#include "../../drv/usb/usb_stm32.h"
#endif

#if PBDRV_CONFIG_UART_STM32H7_LL_DMA
#include <drv/uart/uart_stm32h7_ll_dma.h>
#include <stm32h7xx_ll_dma.h>
#endif

#if PBDRV_CONFIG_UART_STM32H7_LL_IRQ
#include <drv/uart/uart_stm32h7_ll_irq.h>
#endif

#include <stdbool.h>

#include <stm32h743xx.h>
#include <stm32h7xx_ll_lpuart.h>

#if PBDRV_CONFIG_USB_STM32H7
#endif

enum {
    UART_PORT_A,
    UART_PORT_B,
    UART_PORT_C,
    UART_PORT_D,
    UART_PORT_E,
    UART_PORT_F,
    UART_PORT_G,
    UART_PORT_H,
};

volatile uint32_t peak_lpuart1_irq_count;
volatile uint32_t peak_lpuart1_rx_count;
volatile uint32_t peak_lpuart1_tx_count;
volatile uint32_t peak_lpuart1_hci_reset_sent_count;
volatile uint32_t peak_lpuart1_hci_reset_rsp_count;
volatile uint32_t peak_lpuart1_hci_last_byte;
volatile uint32_t peak_lpuart1_cts_asserted_count;
volatile uint32_t peak_lpuart1_cts_deasserted_count;
volatile uint32_t peak_lpuart1_tx_blocked_by_cts;
volatile uint32_t peak_lpuart1_rx_log_count;
volatile uint8_t peak_lpuart1_rx_log[32];

static const uint8_t peak_hci_reset_cmd[] = {0x01, 0x03, 0x0C, 0x00};
static uint8_t peak_hci_match_index;

static void lpuart1_write_byte(uint8_t b) {
    bool was_blocked = false;

    for (uint32_t i = 0; i < 200000; i++) {
        #if PBDRV_CONFIG_BLUETOOTH_PEAK_FLOW_PROBE
        bool cts_asserted = (PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PORT->IDR & (1u << PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PIN)) == 0;
        if (!cts_asserted) {
            peak_lpuart1_cts_deasserted_count++;
            was_blocked = true;
            continue;
        }
        peak_lpuart1_cts_asserted_count++;
        #endif

        if (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->ISR & USART_ISR_TXE_TXFNF) {
            PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->TDR = b;
            peak_lpuart1_tx_count++;
            if (was_blocked) {
                peak_lpuart1_tx_blocked_by_cts++;
            }
            return;
        }
    }
}

#if !PBDRV_CONFIG_BLUETOOTH_PEAK_HCI_PROBE
static void lpuart1_write_str(const char *s) {
    while (*s) {
        lpuart1_write_byte((uint8_t)*s++);
    }
}
#endif

static void lpuart1_send_hci_reset(void) {
    for (uint32_t i = 0; i < sizeof(peak_hci_reset_cmd); i++) {
        lpuart1_write_byte(peak_hci_reset_cmd[i]);
    }
    peak_lpuart1_hci_reset_sent_count++;
}

#if PBDRV_CONFIG_UART_STM32H7_LL_IRQ
const pbdrv_uart_stm32h7_ll_irq_platform_data_t
    pbdrv_uart_stm32h7_ll_irq_platform_data[PBDRV_CONFIG_UART_STM32H7_LL_IRQ_NUM_UART] = {
    [UART_PORT_A] = {
        .uart = USART1,
        .irq = USART1_IRQn,
    },
    [UART_PORT_B] = {
        .uart = USART2,
        .irq = USART2_IRQn,
    },
};

#endif

#if PBDRV_CONFIG_UART_STM32H7_LL_DMA

const pbdrv_uart_stm32h7_ll_dma_platform_data_t
    pbdrv_uart_stm32h7_ll_dma_platform_data[PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART] = {
    [UART_PORT_A] = {
        .tx_dma = DMA1,
        .tx_dma_stream = LL_DMA_STREAM_0,
        .tx_dma_req = LL_DMAMUX1_REQ_USART1_TX,
        .tx_dma_irq = DMA1_Stream0_IRQn,
        .rx_dma = DMA1,
        .rx_dma_stream = LL_DMA_STREAM_1,
        .rx_dma_req = LL_DMAMUX1_REQ_USART1_RX,
        .rx_dma_irq = DMA1_Stream1_IRQn,
        .uart = USART1,
        .uart_irq = USART1_IRQn,
    },
    [UART_PORT_B] = {
        .tx_dma = DMA1,
        .tx_dma_stream = LL_DMA_STREAM_2,
        .tx_dma_req = LL_DMAMUX1_REQ_USART2_TX,
        .tx_dma_irq = DMA1_Stream2_IRQn,
        .rx_dma = DMA1,
        .rx_dma_stream = LL_DMA_STREAM_3,
        .rx_dma_req = LL_DMAMUX1_REQ_USART2_RX,
        .rx_dma_irq = DMA1_Stream3_IRQn,
        .uart = USART2,
        .uart_irq = USART2_IRQn,
    },
    [UART_PORT_C] = {
        .tx_dma = DMA1,
        .tx_dma_stream = LL_DMA_STREAM_4,
        .tx_dma_req = LL_DMAMUX1_REQ_USART3_TX,
        .tx_dma_irq = DMA1_Stream4_IRQn,
        .rx_dma = DMA1,
        .rx_dma_stream = LL_DMA_STREAM_5,
        .rx_dma_req = LL_DMAMUX1_REQ_USART3_RX,
        .rx_dma_irq = DMA1_Stream5_IRQn,
        .uart = USART3,
        .uart_irq = USART3_IRQn,
    },
    [UART_PORT_D] = {
        .tx_dma = DMA1,
        .tx_dma_stream = LL_DMA_STREAM_6,
        .tx_dma_req = LL_DMAMUX1_REQ_USART6_TX,
        .tx_dma_irq = DMA1_Stream6_IRQn,
        .rx_dma = DMA1,
        .rx_dma_stream = LL_DMA_STREAM_7,
        .rx_dma_req = LL_DMAMUX1_REQ_USART6_RX,
        .rx_dma_irq = DMA1_Stream7_IRQn,
        .uart = USART6,
        .uart_irq = USART6_IRQn,
    },
    [UART_PORT_E] = {
        .tx_dma = DMA2,
        .tx_dma_stream = LL_DMA_STREAM_0,
        .tx_dma_req = LL_DMAMUX1_REQ_UART4_TX,
        .tx_dma_irq = DMA2_Stream0_IRQn,
        .rx_dma = DMA2,
        .rx_dma_stream = LL_DMA_STREAM_1,
        .rx_dma_req = LL_DMAMUX1_REQ_UART4_RX,
        .rx_dma_irq = DMA2_Stream1_IRQn,
        .uart = UART4,
        .uart_irq = UART4_IRQn,
    },
    [UART_PORT_F] = {
        .tx_dma = DMA2,
        .tx_dma_stream = LL_DMA_STREAM_2,
        .tx_dma_req = LL_DMAMUX1_REQ_UART5_TX,
        .tx_dma_irq = DMA2_Stream2_IRQn,
        .rx_dma = DMA2,
        .rx_dma_stream = LL_DMA_STREAM_3,
        .rx_dma_req = LL_DMAMUX1_REQ_UART5_RX,
        .rx_dma_irq = DMA2_Stream3_IRQn,
        .uart = UART5,
        .uart_irq = UART5_IRQn,
    },
    [UART_PORT_G] = {
        .tx_dma = DMA2,
        .tx_dma_stream = LL_DMA_STREAM_4,
        .tx_dma_req = LL_DMAMUX1_REQ_UART7_TX,
        .tx_dma_irq = DMA2_Stream4_IRQn,
        .rx_dma = DMA2,
        .rx_dma_stream = LL_DMA_STREAM_5,
        .rx_dma_req = LL_DMAMUX1_REQ_UART7_RX,
        .rx_dma_irq = DMA2_Stream5_IRQn,
        .uart = UART7,
        .uart_irq = UART7_IRQn,
    },
    [UART_PORT_H] = {
        .tx_dma = DMA2,
        .tx_dma_stream = LL_DMA_STREAM_6,
        .tx_dma_req = LL_DMAMUX1_REQ_UART8_TX,
        .tx_dma_irq = DMA2_Stream6_IRQn,
        .rx_dma = DMA2,
        .rx_dma_stream = LL_DMA_STREAM_7,
        .rx_dma_req = LL_DMAMUX1_REQ_UART8_RX,
        .rx_dma_irq = DMA2_Stream7_IRQn,
        .uart = UART8,
        .uart_irq = UART8_IRQn,
    },
};

#endif

void USART1_IRQHandler(void) {
#if PBDRV_CONFIG_UART_STM32H7_LL_DMA
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_A);
#elif PBDRV_CONFIG_UART_STM32H7_LL_IRQ
    pbdrv_uart_stm32h7_ll_irq_handle_irq(UART_PORT_A);
#endif
}

void LPUART1_IRQHandler(void) {
    uint32_t isr = PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->ISR;

    if (isr & USART_ISR_PE) {
        PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->ICR = USART_ICR_PECF;
    }
    if (isr & USART_ISR_FE) {
        PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->ICR = USART_ICR_FECF;
    }
    if (isr & USART_ISR_NE) {
        PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->ICR = USART_ICR_NECF;
    }
    if (isr & USART_ISR_ORE) {
        PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->ICR = USART_ICR_ORECF;
    }

    if (isr & USART_ISR_RXNE_RXFNE) {
        uint8_t c = (uint8_t)PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->RDR;
        peak_lpuart1_rx_count++;
        peak_lpuart1_hci_last_byte = c;

        if (peak_lpuart1_rx_log_count < sizeof(peak_lpuart1_rx_log)) {
            peak_lpuart1_rx_log[peak_lpuart1_rx_log_count++] = c;
        }

        #if PBDRV_CONFIG_BLUETOOTH_PEAK_HCI_PROBE
        switch (peak_hci_match_index) {
            case 0:
                peak_hci_match_index = (c == 0x04) ? 1 : 0; // HCI Event packet type
                break;
            case 1:
                peak_hci_match_index = (c == 0x0E) ? 2 : (c == 0x04 ? 1 : 0); // Command Complete
                break;
            case 2:
                peak_hci_match_index = (c == 0x04) ? 3 : (c == 0x04 ? 1 : 0); // parameter length
                break;
            case 3:
                // Num_HCI_Command_Packets can vary by controller; accept any.
                peak_hci_match_index = 4;
                break;
            case 4:
                peak_hci_match_index = (c == 0x03) ? 5 : (c == 0x04 ? 1 : 0); // OCF(HCI Reset)
                break;
            case 5:
                peak_hci_match_index = (c == 0x0C) ? 6 : (c == 0x04 ? 1 : 0); // OGF(HCI Reset)
                break;
            case 6:
                if (c == 0x00) {
                    peak_lpuart1_hci_reset_rsp_count++;
                }
                peak_hci_match_index = (c == 0x04) ? 1 : 0;
                break;
            default:
                peak_hci_match_index = 0;
                break;
        }
        #else
        if (c == '\r') {
            lpuart1_write_str("\r\n");
        } else {
            lpuart1_write_byte(c);
        }
        #endif
    }

    peak_lpuart1_irq_count++;
}

#if PBDRV_CONFIG_USB_STM32H7
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd) {
    GPIO_InitTypeDef gpio_init;

    if (hpcd->Instance != USB_OTG_FS) {
        return;
    }

    gpio_init.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    #if defined(USB2_OTG_FS)
    gpio_init.Alternate = GPIO_AF10_OTG2_FS;
    #else
    gpio_init.Alternate = GPIO_AF10_OTG1_FS;
    #endif
    HAL_GPIO_Init(GPIOA, &gpio_init);

    #if defined(USB2_OTG_FS)
    __HAL_RCC_USB2_OTG_FS_CLK_ENABLE();
    #else
    __HAL_RCC_USB1_OTG_HS_CLK_ENABLE();
    #endif

    // Keep USB2 OTG FS clock running during sleep (WFI), otherwise the main
    // loop's wait-for-interrupt will gate the AHB1 clock and drop USB traffic.
    __HAL_RCC_USB2_OTG_FS_CLK_SLEEP_ENABLE();
    __HAL_RCC_USB2_OTG_FS_ULPI_CLK_SLEEP_DISABLE();

    HAL_NVIC_SetPriority(OTG_FS_EP1_OUT_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_EP1_OUT_IRQn);
    HAL_NVIC_SetPriority(OTG_FS_EP1_IN_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_EP1_IN_IRQn);
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    pbdrv_usb_stm32_handle_vbus_irq(true);
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd) {
    if (hpcd->Instance != USB_OTG_FS) {
        return;
    }

    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
}

void OTG_FS_IRQHandler(void) {
    pbdrv_usb_stm32_handle_otg_fs_irq();
}

void OTG_FS_EP1_OUT_IRQHandler(void) {
    pbdrv_usb_stm32_handle_otg_fs_irq();
}

void OTG_FS_EP1_IN_IRQHandler(void) {
    pbdrv_usb_stm32_handle_otg_fs_irq();
}

#endif

#if PBDRV_CONFIG_UART_STM32H7_LL_DMA
void USART2_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_B);
}

void USART3_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_C);
}

void USART6_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_D);
}

void UART4_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_E);
}

void UART5_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_F);
}

void UART7_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_G);
}

void UART8_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_H);
}
#elif PBDRV_CONFIG_UART_STM32H7_LL_IRQ
void USART2_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_irq_handle_irq(UART_PORT_B);
}
#endif

#if PBDRV_CONFIG_UART_STM32H7_LL_DMA
void DMA1_Stream0_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_A);
}

void DMA1_Stream1_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_A);
}

void DMA1_Stream2_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_B);
}

void DMA1_Stream3_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_B);
}

void DMA1_Stream4_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_C);
}

void DMA1_Stream5_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_C);
}

void DMA1_Stream6_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_D);
}

void DMA1_Stream7_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_D);
}

void DMA2_Stream0_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_E);
}

void DMA2_Stream1_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_E);
}

void DMA2_Stream2_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_F);
}

void DMA2_Stream3_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_F);
}

void DMA2_Stream4_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_G);
}

void DMA2_Stream5_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_G);
}

void DMA2_Stream6_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_tx_dma_irq(UART_PORT_H);
}

void DMA2_Stream7_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_dma_handle_rx_dma_irq(UART_PORT_H);
}
#endif

uint32_t SystemCoreClock = PBDRV_CONFIG_SYS_CLOCK_RATE;
uint32_t SystemD2Clock = PBDRV_CONFIG_SYS_CLOCK_RATE;
const uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

__attribute__((section(".name"), used))
const char pbdrv_hub_name[] = "Peak";

extern uint32_t *_fw_isr_vector_src;

static void configure_gpio_for_uart(void) {
    // PB14/PB15 -> USART1 TX/RX (AF4)
    GPIOB->MODER &= ~((3u << (14 * 2)) | (3u << (15 * 2)));
    GPIOB->MODER |=  ((2u << (14 * 2)) | (2u << (15 * 2)));
    GPIOB->OSPEEDR |= (3u << (14 * 2)) | (3u << (15 * 2));
    GPIOB->PUPDR &= ~((3u << (14 * 2)) | (3u << (15 * 2)));
    GPIOB->PUPDR |=  (1u << (15 * 2));
    GPIOB->AFR[1] &= ~((0xFu << ((14 - 8) * 4)) | (0xFu << ((15 - 8) * 4)));
    GPIOB->AFR[1] |=  ((4u << ((14 - 8) * 4)) | (4u << ((15 - 8) * 4)));

    // PD5/PD6 -> USART2 TX/RX (AF7)
    GPIOD->MODER &= ~((3u << (5 * 2)) | (3u << (6 * 2)));
    GPIOD->MODER |=  ((2u << (5 * 2)) | (2u << (6 * 2)));
    GPIOD->OSPEEDR |= (3u << (5 * 2)) | (3u << (6 * 2));
    GPIOD->PUPDR &= ~((3u << (5 * 2)) | (3u << (6 * 2)));
    GPIOD->PUPDR |=  (1u << (6 * 2));
    GPIOD->AFR[0] &= ~((0xFu << (5 * 4)) | (0xFu << (6 * 4)));
    GPIOD->AFR[0] |=  ((7u << (5 * 4)) | (7u << (6 * 4)));

    // PB10/PB11 -> USART3 TX/RX (AF7)
    GPIOB->MODER &= ~((3u << (10 * 2)) | (3u << (11 * 2)));
    GPIOB->MODER |=  ((2u << (10 * 2)) | (2u << (11 * 2)));
    GPIOB->OSPEEDR |= (3u << (10 * 2)) | (3u << (11 * 2));
    GPIOB->PUPDR &= ~((3u << (10 * 2)) | (3u << (11 * 2)));
    GPIOB->PUPDR |=  (1u << (11 * 2));
    GPIOB->AFR[1] &= ~((0xFu << ((10 - 8) * 4)) | (0xFu << ((11 - 8) * 4)));
    GPIOB->AFR[1] |=  ((7u << ((10 - 8) * 4)) | (7u << ((11 - 8) * 4)));

    // PC6/PC7 -> USART6 TX/RX (AF7)
    GPIOC->MODER &= ~((3u << (6 * 2)) | (3u << (7 * 2)));
    GPIOC->MODER |=  ((2u << (6 * 2)) | (2u << (7 * 2)));
    GPIOC->OSPEEDR |= (3u << (6 * 2)) | (3u << (7 * 2));
    GPIOC->PUPDR &= ~((3u << (6 * 2)) | (3u << (7 * 2)));
    GPIOC->PUPDR |=  (1u << (7 * 2));
    GPIOC->AFR[0] &= ~((0xFu << (6 * 4)) | (0xFu << (7 * 4)));
    GPIOC->AFR[0] |=  ((7u << (6 * 4)) | (7u << (7 * 4)));

    // PC10/PC11 -> UART4 TX/RX (AF8)
    GPIOC->MODER &= ~((3u << (10 * 2)) | (3u << (11 * 2)));
    GPIOC->MODER |=  ((2u << (10 * 2)) | (2u << (11 * 2)));
    GPIOC->OSPEEDR |= (3u << (10 * 2)) | (3u << (11 * 2));
    GPIOC->PUPDR &= ~((3u << (10 * 2)) | (3u << (11 * 2)));
    GPIOC->PUPDR |=  (1u << (11 * 2));
    GPIOC->AFR[1] &= ~((0xFu << ((10 - 8) * 4)) | (0xFu << ((11 - 8) * 4)));
    GPIOC->AFR[1] |=  ((8u << ((10 - 8) * 4)) | (8u << ((11 - 8) * 4)));

    // PB13/PB12 -> UART5 TX/RX (AF14)
    GPIOB->MODER &= ~((3u << (13 * 2)) | (3u << (12 * 2)));
    GPIOB->MODER |=  ((2u << (13 * 2)) | (2u << (12 * 2)));
    GPIOB->OSPEEDR |= (3u << (13 * 2)) | (3u << (12 * 2));
    GPIOB->PUPDR &= ~((3u << (13 * 2)) | (3u << (12 * 2)));
    GPIOB->PUPDR |=  (1u << (12 * 2));
    GPIOB->AFR[1] &= ~((0xFu << ((13 - 8) * 4)) | (0xFu << ((12 - 8) * 4)));
    GPIOB->AFR[1] |=  ((14u << ((13 - 8) * 4)) | (14u << ((12 - 8) * 4)));

    // PE8/PE7 -> UART7 TX/RX (AF7)
    GPIOE->MODER &= ~((3u << (8 * 2)) | (3u << (7 * 2)));
    GPIOE->MODER |=  ((2u << (8 * 2)) | (2u << (7 * 2)));
    GPIOE->OSPEEDR |= (3u << (8 * 2)) | (3u << (7 * 2));
    GPIOE->PUPDR &= ~((3u << (8 * 2)) | (3u << (7 * 2)));
    GPIOE->PUPDR |=  (1u << (7 * 2));
    GPIOE->AFR[0] &= ~(0xFu << (7 * 4));
    GPIOE->AFR[0] |=  (7u << (7 * 4));
    GPIOE->AFR[1] &= ~(0xFu << ((8 - 8) * 4));
    GPIOE->AFR[1] |=  (7u << ((8 - 8) * 4));

    // PE1/PE0 -> UART8 TX/RX (AF8)
    GPIOE->MODER &= ~((3u << (1 * 2)) | (3u << (0 * 2)));
    GPIOE->MODER |=  ((2u << (1 * 2)) | (2u << (0 * 2)));
    GPIOE->OSPEEDR |= (3u << (1 * 2)) | (3u << (0 * 2));
    GPIOE->PUPDR &= ~((3u << (1 * 2)) | (3u << (0 * 2)));
    GPIOE->PUPDR |=  (1u << (0 * 2));
    GPIOE->AFR[0] &= ~((0xFu << (1 * 4)) | (0xFu << (0 * 4)));
    GPIOE->AFR[0] |=  ((8u << (1 * 4)) | (8u << (0 * 4)));
}

static void configure_gpio_for_lpuart1(void) {
    // PB6/PB7 -> LPUART1 TX/RX (AF8), smoke-test mapping.
    GPIOB->MODER &= ~((3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_TX_PIN * 2)) | (3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN * 2)));
    GPIOB->MODER |=  ((2u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_TX_PIN * 2)) | (2u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN * 2)));
    GPIOB->OSPEEDR |= (3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_TX_PIN * 2)) | (3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN * 2));
    GPIOB->PUPDR &= ~((3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_TX_PIN * 2)) | (3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN * 2)));
    GPIOB->PUPDR |=  (1u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN * 2));
    GPIOB->AFR[0] &= ~((0xFu << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_TX_PIN * 4)) | (0xFu << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN * 4)));
    GPIOB->AFR[0] |=  ((8u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_TX_PIN * 4)) | (8u << (PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN * 4)));
}

static void configure_lpuart1_smoke(void) {
    PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->CR1 = 0;
    PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->CR2 = 0;
    PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->CR3 = 0;

    LL_LPUART_SetBaudRate(PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE, PBDRV_CONFIG_SYS_CLOCK_RATE, LL_LPUART_PRESCALER_DIV1, PBDRV_CONFIG_BLUETOOTH_PEAK_UART_BAUD);

    PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE | USART_CR1_UE;

    #if PBDRV_CONFIG_BLUETOOTH_PEAK_HCI_PROBE
    lpuart1_send_hci_reset();
    #else
    lpuart1_write_str("\r\n[LPUART1 smoke]\r\n");
    #endif

    NVIC_SetPriority(PBDRV_CONFIG_BLUETOOTH_PEAK_UART_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
    NVIC_EnableIRQ(PBDRV_CONFIG_BLUETOOTH_PEAK_UART_IRQ);
}

static void configure_bluetooth_reset_placeholder(void) {
    if (PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PIN == 0xFF) {
        return;
    }

    uint32_t pin = PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PIN;
    PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PORT->MODER &= ~(3u << (pin * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PORT->MODER |=  (1u << (pin * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PORT->OTYPER &= ~(1u << pin);
    PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PORT->OSPEEDR |= (3u << (pin * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PORT->PUPDR &= ~(3u << (pin * 2));

    // Keep coprocessor out of reset by default.
    PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PORT->BSRR = (1u << pin);
}

static void configure_bluetooth_flowcontrol_placeholders(void) {
    // RTS placeholder: output low (asserted/ready) so peer is allowed to send.
    PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PORT->MODER &= ~(3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PIN * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PORT->MODER |=  (1u << (PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PIN * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PORT->OTYPER &= ~(1u << PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PIN);
    PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PORT->OSPEEDR |= (3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PIN * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PORT->PUPDR &= ~(3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PIN * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PORT->BSRR = (1u << (PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PIN + 16));

    // CTS placeholder: input with pull-up.
    PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PORT->MODER &= ~(3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PIN * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PORT->PUPDR &= ~(3u << (PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PIN * 2));
    PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PORT->PUPDR |=  (1u << (PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PIN * 2));
}

static void configure_heartbeat_led(void) {
    // WeAct Mini H7 user LED is on PE3.
    GPIOE->MODER &= ~(3u << (3 * 2));
    GPIOE->MODER |=  (1u << (3 * 2));
    GPIOE->OTYPER &= ~(1u << 3);
    GPIOE->OSPEEDR |= (3u << (3 * 2));
    GPIOE->PUPDR &= ~(3u << (3 * 2));
    GPIOE->BSRR = (1u << (3 + 16));
}

void SystemInit(void) {
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10U * 2U) | (3UL << 11U * 2U));
    #endif

    SCB->CCR |= SCB_CCR_STKALIGN_Msk;

    // Firmware starts at beginning of flash for bring-up.
    SCB->VTOR = (uint32_t)&_fw_isr_vector_src;

    // HSI is 64 MHz after reset; keep default clock tree for first bring-up.
    SystemCoreClock = PBDRV_CONFIG_SYS_CLOCK_RATE;

    // Enable clocks for GPIO banks and UART peripherals used in phase 1.
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN | RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOCEN | RCC_AHB4ENR_GPIODEN | RCC_AHB4ENR_GPIOEEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    RCC->APB1LENR |= RCC_APB1LENR_USART2EN;
    RCC->APB1LENR |= RCC_APB1LENR_USART3EN;
    RCC->APB1LENR |= RCC_APB1LENR_UART4EN | RCC_APB1LENR_UART5EN;
    RCC->APB1LENR |= RCC_APB1LENR_UART7EN | RCC_APB1LENR_UART8EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;
    RCC->APB4ENR |= RCC_APB4ENR_LPUART1EN;

    #if PBDRV_CONFIG_USB_STM32H7
    bool usb_clk_from_pll3 = false;

    // Prefer a crystal-derived 48 MHz USB clock: HSE(25MHz) -> PLL3Q(48MHz).
    RCC->CR |= RCC_CR_HSEON;
    for (uint32_t i = 0; i < 1000000; i++) {
        if (RCC->CR & RCC_CR_HSERDY) {
            RCC->CR &= ~RCC_CR_PLL3ON;
            while (RCC->CR & RCC_CR_PLL3RDY) {
            }

            MODIFY_REG(RCC->PLLCKSELR,
                RCC_PLLCKSELR_PLLSRC | RCC_PLLCKSELR_DIVM3,
                RCC_PLLCKSELR_PLLSRC_HSE | (5u << RCC_PLLCKSELR_DIVM3_Pos));
            MODIFY_REG(RCC->PLL3DIVR,
                RCC_PLL3DIVR_N3 | RCC_PLL3DIVR_P3 | RCC_PLL3DIVR_Q3 | RCC_PLL3DIVR_R3,
                ((96u - 1u) << RCC_PLL3DIVR_N3_Pos)
                | ((2u - 1u) << RCC_PLL3DIVR_P3_Pos)
                | ((10u - 1u) << RCC_PLL3DIVR_Q3_Pos)
                | ((2u - 1u) << RCC_PLL3DIVR_R3_Pos));
            RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ3EN;

            RCC->CR |= RCC_CR_PLL3ON;
            while (!(RCC->CR & RCC_CR_PLL3RDY)) {
            }

            MODIFY_REG(RCC->D2CCIP2R, RCC_D2CCIP2R_USBSEL, RCC_D2CCIP2R_USBSEL_1);
            usb_clk_from_pll3 = true;
            break;
        }
    }

    if (!usb_clk_from_pll3) {
        RCC->CR |= RCC_CR_HSI48ON;
        while (!(RCC->CR & RCC_CR_HSI48RDY)) {
        }
        MODIFY_REG(RCC->D2CCIP2R, RCC_D2CCIP2R_USBSEL, RCC_D2CCIP2R_USBSEL);
    }
    #endif

    configure_gpio_for_uart();
    configure_gpio_for_lpuart1();
    configure_bluetooth_flowcontrol_placeholders();
    configure_bluetooth_reset_placeholder();
    configure_lpuart1_smoke();
    configure_heartbeat_led();

    // 1 ms system tick for pbdrv_clock_stm32.
    SysTick_Config(PBDRV_CONFIG_SYS_CLOCK_RATE / 1000);

    // Ensure interrupts are enabled for SysTick heartbeat.
    __enable_irq();
}
