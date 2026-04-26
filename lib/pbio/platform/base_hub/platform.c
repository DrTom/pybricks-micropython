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

void SystemInit(void) {
    pbdrv_usb_diag_systeminit_calls++;
    pbdrv_usb_diag_stage = 0x0100;

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
