// SPDX-License-Identifier: MIT

#include <pbdrv/config.h>

#include <pbdrv/uart.h>

#if PBDRV_CONFIG_UART_STM32H7_LL_DMA
#include <drv/uart/uart_stm32h7_ll_dma.h>
#include <stm32h7xx_ll_dma.h>
#endif

#if PBDRV_CONFIG_UART_STM32H7_LL_IRQ
#include <drv/uart/uart_stm32h7_ll_irq.h>
#endif

#include <stm32h743xx.h>

enum {
    UART_PORT_A,
#if PBDRV_CONFIG_UART_STM32H7_LL_IRQ
    UART_PORT_B,
#endif
};

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
};

#endif

void USART1_IRQHandler(void) {
#if PBDRV_CONFIG_UART_STM32H7_LL_DMA
    pbdrv_uart_stm32h7_ll_dma_handle_uart_irq(UART_PORT_A);
#elif PBDRV_CONFIG_UART_STM32H7_LL_IRQ
    pbdrv_uart_stm32h7_ll_irq_handle_irq(UART_PORT_A);
#endif
}

#if PBDRV_CONFIG_UART_STM32H7_LL_IRQ
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
#endif

uint32_t SystemCoreClock = PBDRV_CONFIG_SYS_CLOCK_RATE;

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
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIODEN | RCC_AHB4ENR_GPIOEEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB1LENR |= RCC_APB1LENR_USART2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    configure_gpio_for_uart();
    configure_heartbeat_led();

    // 1 ms system tick for pbdrv_clock_stm32.
    SysTick_Config(PBDRV_CONFIG_SYS_CLOCK_RATE / 1000);

    // Ensure interrupts are enabled for SysTick heartbeat.
    __enable_irq();
}
