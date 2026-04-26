// SPDX-License-Identifier: MIT
// Copyright (c) 2022 The Pybricks Authors

// Internal header for STM32 USB driver.

#ifndef _INTERNAL_PBDRV_USB_STM32_H_
#define _INTERNAL_PBDRV_USB_STM32_H_

#include <pbdrv/config.h>

#if PBDRV_CONFIG_USB_STM32F4 || PBDRV_CONFIG_USB_STM32H7 || PBDRV_CONFIG_USB_STM32H5

#include <stdbool.h>
#include <stdint.h>

void pbdrv_usb_stm32_handle_otg_fs_irq(void);
void pbdrv_usb_stm32_handle_vbus_irq(bool active);

#if PBDRV_CONFIG_USB_STM32H5
extern volatile uint32_t pbdrv_usb_diag_magic;
extern volatile uint32_t pbdrv_usb_diag_stage;
extern volatile uint32_t pbdrv_usb_diag_systeminit_calls;
extern volatile uint32_t pbdrv_usb_diag_pbdrv_usb_init_calls;
extern volatile uint32_t pbdrv_usb_diag_usbd_ll_init_calls;
extern volatile uint32_t pbdrv_usb_diag_hal_pcd_msp_init_calls;
extern volatile uint32_t pbdrv_usb_diag_hal_pcd_msp_step;
extern volatile uint32_t pbdrv_usb_diag_hal_pcd_init_status;
extern volatile uint32_t pbdrv_usb_diag_hal_pcd_start_status;
extern volatile uint32_t pbdrv_usb_diag_usbd_start_status;
extern volatile uint32_t pbdrv_usb_diag_usbd_ll_start_calls;
extern volatile uint32_t pbdrv_usb_diag_otg_irq_count;
extern volatile uint32_t pbdrv_usb_diag_poll_irq_count;
extern volatile uint32_t pbdrv_usb_diag_is_ready_calls;
extern volatile uint32_t pbdrv_usb_diag_wait_cfg_calls;
extern volatile uint32_t pbdrv_usb_diag_vbus_irq_count;
extern volatile uint32_t pbdrv_usb_diag_reset_cb_count;
extern volatile uint32_t pbdrv_usb_diag_setup_cb_count;
extern volatile uint32_t pbdrv_usb_diag_data_out_cb_count;
extern volatile uint32_t pbdrv_usb_diag_data_in_cb_count;
extern volatile uint32_t pbdrv_usb_diag_connect_cb_count;
extern volatile uint32_t pbdrv_usb_diag_disconnect_cb_count;
#endif

#endif // PBDRV_CONFIG_USB_STM32F4 || PBDRV_CONFIG_USB_STM32H7 || PBDRV_CONFIG_USB_STM32H5

#endif // _INTERNAL_PBDRV_USB_STM32_H_
