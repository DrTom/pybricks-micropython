// SPDX-License-Identifier: MIT

#include <pbdrv/config.h>

#if PBDRV_CONFIG_USB_STM32H5
#include <pbdrv/usb.h>
#include STM32_HAL_H
#include "../../drv/usb/usb_stm32.h"
#endif

extern uint32_t *_fw_isr_vector_src;

uint32_t SystemCoreClock = PBDRV_CONFIG_SYS_CLOCK_RATE;
const uint8_t AHBPrescTable[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9 };

volatile uint32_t pb_diag_hardfault_count __attribute__((used));
volatile uint32_t pb_diag_hardfault_lr __attribute__((used));
volatile uint32_t pb_diag_hardfault_sp __attribute__((used));
volatile uint32_t pb_diag_hardfault_cfsr __attribute__((used));
volatile uint32_t pb_diag_hardfault_hfsr __attribute__((used));
volatile uint32_t pb_diag_hardfault_bfar __attribute__((used));
volatile uint32_t pb_diag_hardfault_mmfar __attribute__((used));
volatile uint32_t pb_diag_hardfault_cpacr __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_r0 __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_r1 __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_r2 __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_r3 __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_r12 __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_lr __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_pc __attribute__((used));
volatile uint32_t pb_diag_hardfault_stacked_xpsr __attribute__((used));

void pb_diag_hardfault_handler_c(uint32_t *sp, uint32_t lr) __attribute__((used, noinline));
void pb_diag_hardfault_handler_c(uint32_t *sp, uint32_t lr) {
    pb_diag_hardfault_count++;
    pb_diag_hardfault_lr = lr;
    pb_diag_hardfault_sp = (uint32_t)sp;

    pb_diag_hardfault_cfsr = SCB->CFSR;
    pb_diag_hardfault_hfsr = SCB->HFSR;
    pb_diag_hardfault_bfar = SCB->BFAR;
    pb_diag_hardfault_mmfar = SCB->MMFAR;
    pb_diag_hardfault_cpacr = SCB->CPACR;

    if (sp) {
        pb_diag_hardfault_stacked_r0 = sp[0];
        pb_diag_hardfault_stacked_r1 = sp[1];
        pb_diag_hardfault_stacked_r2 = sp[2];
        pb_diag_hardfault_stacked_r3 = sp[3];
        pb_diag_hardfault_stacked_r12 = sp[4];
        pb_diag_hardfault_stacked_lr = sp[5];
        pb_diag_hardfault_stacked_pc = sp[6];
        pb_diag_hardfault_stacked_xpsr = sp[7];
    }

    for (;;) {
    }
}

void HardFault_Handler(void) __attribute__((naked));
void HardFault_Handler(void) {
    __asm volatile (
        "tst   lr, #4              \n"
        "ite   eq                  \n"
        "mrseq r0, msp             \n"
        "mrsne r0, psp             \n"
        "mov   r1, lr              \n"
        "b     pb_diag_hardfault_handler_c \n"
    );
}

void SystemInit(void) {
    pbdrv_usb_diag_systeminit_calls++;
    pbdrv_usb_diag_stage = 0x0100;

    // Enable FP extension access (CP10, CP11 full access) before any code path
    // that may use floating-point registers in assembly (e.g. MicroPython NLR).
    SCB->CPACR |= (0xFu << 20);
    __DSB();
    __ISB();

    SCB->VTOR = (uint32_t)&_fw_isr_vector_src;

    RCC_OscInitTypeDef osc_init = {0};
    osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
    osc_init.HSEState = RCC_HSE_OFF;
    osc_init.HSIState = RCC_HSI_ON;
    osc_init.HSIDiv = RCC_HSI_DIV1;
    osc_init.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc_init.HSI48State = RCC_HSI48_ON;
    osc_init.CSIState = RCC_CSI_OFF;
    osc_init.PLL.PLLState = RCC_PLL_OFF;
    HAL_RCC_OscConfig(&osc_init);

    RCC_ClkInitTypeDef clk_init = {0};
    clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
    clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk_init.AHBCLKDivider = RCC_HCLK_DIV1;
    clk_init.APB1CLKDivider = RCC_HCLK_DIV1;
    clk_init.APB2CLKDivider = RCC_HCLK_DIV1;
    clk_init.APB3CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_2);

    SystemCoreClock = PBDRV_CONFIG_SYS_CLOCK_RATE;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    pbdrv_usb_diag_stage = 0x01FF;
}

#if PBDRV_CONFIG_USB_STM32H5
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd) {
    if (hpcd->Instance != USB_DRD_FS) {
        return;
    }

    pbdrv_usb_diag_hal_pcd_msp_init_calls++;
    pbdrv_usb_diag_hal_pcd_msp_step = 0xA001;

    // Ensure HSI48 is enabled before selecting it as USB clock source.
    RCC->CR |= RCC_CR_HSI48ON;
    while (!(RCC->CR & RCC_CR_HSI48RDY)) {
    }

    RCC_PeriphCLKInitTypeDef periph_clk = {0};
    periph_clk.PeriphClockSelection = RCC_PERIPHCLK_USB;
    periph_clk.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    HAL_RCCEx_PeriphCLKConfig(&periph_clk);
    pbdrv_usb_diag_hal_pcd_msp_step = 0xA002;

    HAL_PWREx_EnableVddUSB();
    HAL_PWREx_EnableUSBVoltageDetector();
    HAL_PWREx_DisableUCPDDeadBattery();
    pbdrv_usb_diag_hal_pcd_msp_step = 0xA003;

    __HAL_RCC_USB_CLK_ENABLE();
    pbdrv_usb_diag_hal_pcd_msp_step = 0xA004;

    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = GPIO_AF10_USB;
    HAL_GPIO_Init(GPIOA, &gpio_init);
    pbdrv_usb_diag_hal_pcd_msp_step = 0xA005;

    HAL_NVIC_SetPriority(USB_DRD_FS_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_DRD_FS_IRQn);
    pbdrv_usb_diag_hal_pcd_msp_step = 0xA006;

    pbdrv_usb_stm32_handle_vbus_irq(true);
    pbdrv_usb_diag_hal_pcd_msp_step = 0xA0FF;
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd) {
    if (hpcd->Instance != USB_DRD_FS) {
        return;
    }

    HAL_NVIC_DisableIRQ(USB_DRD_FS_IRQn);
}

void USB_DRD_FS_IRQHandler(void) {
    pbdrv_usb_stm32_handle_otg_fs_irq();
}
#endif
