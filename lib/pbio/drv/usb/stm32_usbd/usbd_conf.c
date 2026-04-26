/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Src/usbd_conf.c
  * @author  MCD Application Team
  * @brief   This file implements the USB Device library callbacks and MSP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include <pbdrv/config.h>
#include STM32_HAL_H
#include "../usb_stm32.h"
#include "usbd_core.h"

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/

/**
  * @brief  Setup stage callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd) {
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_setup_cb_count++;
    #endif
    USBD_LL_SetupStage(hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_data_out_cb_count++;
    #endif
    USBD_LL_DataOutStage(hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Data In stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_data_in_cb_count++;
    #endif
    USBD_LL_DataInStage(hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd) {
    USBD_LL_SOF(hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd) {
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_reset_cb_count++;
    #endif
    USBD_LL_SetSpeed(hpcd->pData, USBD_SPEED_FULL);
    USBD_LL_Reset(hpcd->pData);

#if PBDRV_CONFIG_USB_STM32H7
    // Re-enable RXFLVL interrupt after reset — HAL USBRST handler masks it.
    hpcd->Instance->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;
    // Explicitly (re)arm EP0 OUT setup reception on H7.
    USB_EP0_OutStart(hpcd->Instance, (uint8_t)hpcd->Init.dma_enable, (uint8_t *)hpcd->Setup);
#endif
}

/**
  * @brief  Suspend callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd) {
    USBD_LL_Suspend(hpcd->pData);
}

/**
  * @brief  Resume callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd) {
    USBD_LL_Resume(hpcd->pData);
}

/**
  * @brief  ISOC Out Incomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    USBD_LL_IsoOUTIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  ISOC In Incomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
    USBD_LL_IsoINIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_connect_cb_count++;
    #endif
    USBD_LL_DevConnected(hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd) {
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_disconnect_cb_count++;
    #endif
    USBD_LL_DevDisconnected(hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef  USBD_LL_Init(USBD_HandleTypeDef *pdev) {
    PCD_HandleTypeDef *hpcd = pdev->pData;
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_usbd_ll_init_calls++;
    pbdrv_usb_diag_stage = 0x2000;
    #endif
    /*Set LL Driver parameters */
    #if PBDRV_CONFIG_USB_STM32H5
    hpcd->Instance = USB_DRD_FS;
    #else
    hpcd->Instance = USB_OTG_FS;
    #endif
    #if PBDRV_CONFIG_USB_STM32H5
    hpcd->Init.dev_endpoints = 8;
    #else
    hpcd->Init.dev_endpoints = 9;
    hpcd->Init.use_dedicated_ep1 = 0;
    #endif
    hpcd->Init.ep0_mps = 0x40;
    hpcd->Init.dma_enable = 0;
    hpcd->Init.low_power_enable = 0;
    hpcd->Init.lpm_enable = 0;
    hpcd->Init.battery_charging_enable = 0;
    hpcd->Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd->Init.Sof_enable = 0;
    hpcd->Init.speed = PCD_SPEED_FULL;
    hpcd->Init.vbus_sensing_enable = 0;
    #if PBDRV_CONFIG_USB_STM32H5
    hpcd->Init.bulk_doublebuffer_enable = 0;
    hpcd->Init.iso_singlebuffer_enable = 0;
    #else
    hpcd->Init.use_external_vbus = 0;
    #endif

    /*Initialize LL Driver */
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_stage = 0x2100;
    pbdrv_usb_diag_hal_pcd_init_status = HAL_PCD_Init(hpcd);
    #else
    HAL_PCD_Init(hpcd);
    #endif

    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_stage = 0x2200;
    // Configure PMA for EP0 control and EP1 IN/OUT bulk endpoints.
    HAL_PCDEx_PMAConfig(hpcd, 0x00, PCD_SNG_BUF, 0x18);
    HAL_PCDEx_PMAConfig(hpcd, 0x80, PCD_SNG_BUF, 0x58);
    HAL_PCDEx_PMAConfig(hpcd, 0x01, PCD_SNG_BUF, 0x98);
    HAL_PCDEx_PMAConfig(hpcd, 0x81, PCD_SNG_BUF, 0xD8);
    pbdrv_usb_diag_stage = 0x2300;
    #else
    // H7 USB2 OTG (HS core on FS pins) has >=4 KB dedicated FIFO RAM.
    // Match the ROM DFU bootloader's RX FIFO allocation (0x200 words).
    // TX FIFOs follow immediately after RX.
    HAL_PCDEx_SetRxFiFo(hpcd, 0x200);
    HAL_PCDEx_SetTxFiFo(hpcd, 0, 0x40);  // EP0 IN: 64 words (256 B)
    HAL_PCDEx_SetTxFiFo(hpcd, 1, 0x80);  // EP1 IN: 128 words (512 B)
    #endif

    #if PBDRV_CONFIG_USB_STM32H7
    // Ensure RXFLVL interrupt is enabled - HAL may leave it masked.
    hpcd->Instance->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;
    #endif

    return USBD_OK;
}

/**
  * @brief  De-Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) {
    HAL_PCD_DeInit(pdev->pData);
    return USBD_OK;
}

/**
  * @brief  Starts the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev) {
    #if PBDRV_CONFIG_USB_STM32H5
    pbdrv_usb_diag_usbd_ll_start_calls++;
    pbdrv_usb_diag_stage = 0x2400;
    pbdrv_usb_diag_hal_pcd_start_status = HAL_PCD_Start(pdev->pData);
    // Make sure the USB function is enabled (DADDR.EF = 1) before traffic.
    // On STM32H5, HAL sets this in reset handling, but forcing it here avoids
    // a dead start if reset is never observed.
    HAL_PCD_SetAddress(pdev->pData, 0);
    pbdrv_usb_diag_stage = 0x2500;
    #else
    HAL_PCD_Start(pdev->pData);
    #endif
    return USBD_OK;
}

/**
  * @brief  Stops the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev) {
    HAL_PCD_Stop(pdev->pData);
    return USBD_OK;
}

/**
  * @brief  Opens an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  ep_type: Endpoint Type
  * @param  ep_mps: Endpoint Max Packet Size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev,
    uint8_t ep_addr,
    uint8_t ep_type,
    uint16_t ep_mps) {
    HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);

    return USBD_OK;
}

/**
  * @brief  Closes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    HAL_PCD_EP_Close(pdev->pData, ep_addr);
    return USBD_OK;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    HAL_PCD_EP_Flush(pdev->pData, ep_addr);
    return USBD_OK;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    HAL_PCD_EP_SetStall(pdev->pData, ep_addr);
    return USBD_OK;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);
    return USBD_OK;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Stall (1: yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    PCD_HandleTypeDef *hpcd = pdev->pData;

    if ((ep_addr & 0x80) == 0x80) {
        return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
    } else {
        return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
    }
}

/**
  * @brief  Assigns an USB address to the device
  * @param  pdev: Device handle
  * @param  dev_addr: USB address
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr) {
    HAL_PCD_SetAddress(pdev->pData, dev_addr);
    return USBD_OK;
}

/**
  * @brief  Transmits data over an endpoint
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev,
    uint8_t ep_addr, uint8_t *pbuf, uint32_t size) {
    HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);
    return USBD_OK;
}

/**
  * @brief  Prepares an endpoint for reception
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf:pointer to data to be received
  * @param  size: data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev,
    uint8_t ep_addr, uint8_t *pbuf, uint32_t size) {
    HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);
    return USBD_OK;
}

/**
  * @brief  Returns the last transferred packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Received Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    return HAL_PCD_EP_GetRxCount(pdev->pData, ep_addr);
}
