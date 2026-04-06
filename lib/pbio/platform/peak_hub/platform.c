// SPDX-License-Identifier: MIT

#include <pbdrv/config.h>

#include <pbdrv/uart.h>

#include <drv/uart/uart_stm32h7_ll_irq.h>

#include <stm32h743xx.h>

enum {
    UART_PORT_A,
    UART_PORT_B,
};

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

void USART1_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_irq_handle_irq(UART_PORT_A);
}

void USART2_IRQHandler(void) {
    pbdrv_uart_stm32h7_ll_irq_handle_irq(UART_PORT_B);
}

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
    GPIOB->AFR[1] &= ~((0xFu << ((14 - 8) * 4)) | (0xFu << ((15 - 8) * 4)));
    GPIOB->AFR[1] |=  ((4u << ((14 - 8) * 4)) | (4u << ((15 - 8) * 4)));

    // PD5/PD6 -> USART2 TX/RX (AF7)
    GPIOD->MODER &= ~((3u << (5 * 2)) | (3u << (6 * 2)));
    GPIOD->MODER |=  ((2u << (5 * 2)) | (2u << (6 * 2)));
    GPIOD->OSPEEDR |= (3u << (5 * 2)) | (3u << (6 * 2));
    GPIOD->PUPDR &= ~((3u << (5 * 2)) | (3u << (6 * 2)));
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

static void led_startup_blink(void) {
    // Early bring-up indicator independent of SysTick.
    for (int i = 0; i < 4; i++) {
        GPIOE->ODR ^= (1u << 3);
        for (volatile uint32_t d = 0; d < 3000000; d++) {
        }
    }
}

static void led_fault_blink(uint32_t on, uint32_t off) {
    while (1) {
        GPIOE->BSRR = (1u << 3);
        for (volatile uint32_t d = 0; d < on; d++) {
        }
        GPIOE->BSRR = (1u << (3 + 16));
        for (volatile uint32_t d = 0; d < off; d++) {
        }
    }
}

void HardFault_Handler(void) {
    // Fast blink indicates crash/fault after early boot.
    led_fault_blink(500000, 250000);
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

    configure_gpio_for_uart();
    configure_heartbeat_led();
    led_startup_blink();

    // 1 ms system tick for pbdrv_clock_stm32.
    SysTick_Config(PBDRV_CONFIG_SYS_CLOCK_RATE / 1000);

    // Ensure interrupts are enabled for SysTick heartbeat.
    __enable_irq();
}
