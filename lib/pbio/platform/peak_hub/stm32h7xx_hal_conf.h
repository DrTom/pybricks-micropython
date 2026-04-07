// SPDX-License-Identifier: MIT

#ifndef STM32H7XX_HAL_CONF_H
#define STM32H7XX_HAL_CONF_H

#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DAC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_USART_MODULE_ENABLED

#if !defined(CSI_VALUE)
#define CSI_VALUE ((uint32_t)4000000)
#endif

#if !defined(HSI_VALUE)
#define HSI_VALUE ((uint32_t)64000000)
#endif

#if !defined(LSE_VALUE)
#define LSE_VALUE ((uint32_t)32768)
#endif

#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT ((uint32_t)5000)
#endif

#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT ((uint32_t)5000)
#endif

#if !defined(LSI_VALUE)
#define LSI_VALUE ((uint32_t)32000)
#endif

#if !defined(EXTERNAL_CLOCK_VALUE)
#define EXTERNAL_CLOCK_VALUE ((uint32_t)12288000)
#endif

#define TICK_INT_PRIORITY ((uint32_t)0x00)
#define USE_RTOS 0
#define USE_SD_TRANSCEIVER 0
#define USE_SPI_CRC 1

#include "stm32h7xx_hal_rcc.h"
#include "stm32h7xx_hal_rcc_ex.h"
#include "stm32h7xx_hal_gpio.h"
#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_adc.h"
#include "stm32h7xx_hal_dac.h"
#include "stm32h7xx_hal_flash.h"
#include "stm32h7xx_hal_flash_ex.h"
#include "stm32h7xx_hal_i2c.h"
#include "stm32h7xx_hal_cortex.h"
#include "stm32h7xx_hal_pcd.h"
#include "stm32h7xx_hal_pcd_ex.h"
#include "stm32h7xx_hal_pwr.h"
#include "stm32h7xx_hal_spi.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_hal_uart_ex.h"
#include "stm32h7xx_hal_usart.h"

#define assert_param(expr) ((void)0)

#endif // STM32H7XX_HAL_CONF_H
