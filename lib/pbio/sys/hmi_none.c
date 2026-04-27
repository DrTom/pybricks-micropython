// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2025 The Pybricks Authors

// Provides Human Machine Interface (HMI) between hub and user for Powered Up
// hubs with BLE, lights, and one or more buttons.

#include <pbsys/config.h>

#if PBSYS_CONFIG_HMI_NONE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <pbio/button.h>
#include <pbio/os.h>
#include <pbdrv/bluetooth.h>
#include <pbdrv/usb.h>
#include <pbsys/main.h>
#include <pbsys/status.h>

#ifndef PBSYS_CONFIG_HMI_NONE_STARTS_REPL
#define PBSYS_CONFIG_HMI_NONE_STARTS_REPL (1)
#endif

#ifndef PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE
#define PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE (0)
#endif

#ifndef PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE_PERIOD_MS
#define PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE_PERIOD_MS (250)
#endif

#ifndef PBSYS_CONFIG_HMI_NONE_USB_SUBSCRIBE_STARTS_REPL
#define PBSYS_CONFIG_HMI_NONE_USB_SUBSCRIBE_STARTS_REPL (0)
#endif

volatile uint32_t pbsys_diag_hmi_none_usb_auto_repl_request_calls;
volatile uint32_t pbsys_diag_hmi_none_usb_auto_repl_request_success;
volatile uint32_t pbsys_diag_hmi_none_usb_auto_repl_request_fail;
volatile uint32_t pbsys_diag_hmi_none_usb_auto_repl_last_result;

#if PBSYS_CONFIG_HMI_NONE_USB_SUBSCRIBE_STARTS_REPL
static bool pbsys_hmi_none_usb_auto_repl_requested;
#endif

static void pbsys_hmi_host_update_indications(void) {
    if (pbdrv_usb_connection_is_active()) {
        pbsys_status_set(PBIO_PYBRICKS_STATUS_USB_HOST_CONNECTED);
    } else {
        pbsys_status_clear(PBIO_PYBRICKS_STATUS_USB_HOST_CONNECTED);
    }

    if (pbdrv_bluetooth_host_is_connected()) {
        pbsys_status_set(PBIO_PYBRICKS_STATUS_BLE_HOST_CONNECTED);
    } else {
        pbsys_status_clear(PBIO_PYBRICKS_STATUS_BLE_HOST_CONNECTED);
    }
}

static void pbsys_hmi_connection_changed_callback(void) {
    pbsys_hmi_host_update_indications();

    #if PBSYS_CONFIG_HMI_NONE_USB_SUBSCRIBE_STARTS_REPL
    if (pbdrv_usb_connection_is_active()) {
        if (!pbsys_hmi_none_usb_auto_repl_requested) {
            pbsys_diag_hmi_none_usb_auto_repl_request_calls++;
            pbio_error_t err = pbsys_main_program_request_start(PBIO_PYBRICKS_USER_PROGRAM_ID_REPL, PBSYS_MAIN_PROGRAM_START_REQUEST_TYPE_REMOTE);
            pbsys_diag_hmi_none_usb_auto_repl_last_result = err;
            if (err == PBIO_SUCCESS) {
                pbsys_diag_hmi_none_usb_auto_repl_request_success++;
            } else {
                pbsys_diag_hmi_none_usb_auto_repl_request_fail++;
            }
            pbsys_hmi_none_usb_auto_repl_requested = true;
        }
    } else {
        pbsys_hmi_none_usb_auto_repl_requested = false;
    }
    #endif
}

void pbsys_hmi_init(void) {
    pbdrv_usb_set_host_connection_changed_callback(pbsys_hmi_connection_changed_callback);
    pbdrv_bluetooth_set_host_connection_changed_callback(pbsys_hmi_connection_changed_callback);
    pbsys_hmi_host_update_indications();
}

void pbsys_hmi_deinit(void) {
    pbdrv_usb_set_host_connection_changed_callback(NULL);
    pbdrv_bluetooth_set_host_connection_changed_callback(NULL);
    pbsys_status_clear(PBIO_PYBRICKS_STATUS_USB_HOST_CONNECTED);
    pbsys_status_clear(PBIO_PYBRICKS_STATUS_BLE_HOST_CONNECTED);
}

void pbsys_hmi_stop_animation(void) {
}

pbio_error_t pbsys_hmi_await_program_selection(void) {

    static pbio_os_timer_t auto_advertise_timer;
    pbio_os_timer_set(&auto_advertise_timer, 0);

    do {
        #if PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE
        if (pbio_os_timer_is_expired(&auto_advertise_timer)) {
            if (!pbdrv_bluetooth_host_is_connected()) {
                pbdrv_bluetooth_start_advertising(true);
            }
            pbio_os_timer_set(&auto_advertise_timer, PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE_PERIOD_MS);
        }
        #endif

        if (pbsys_status_test(PBIO_PYBRICKS_STATUS_SHUTDOWN_REQUEST)) {
            return PBIO_ERROR_CANCELED;
        }
        pbio_os_run_processes_and_wait_for_event();
    } while (pbdrv_button_get_pressed());

    #if PBSYS_CONFIG_HMI_NONE_STARTS_REPL
    return pbsys_main_program_request_start(PBIO_PYBRICKS_USER_PROGRAM_ID_REPL, PBSYS_MAIN_PROGRAM_START_REQUEST_TYPE_BOOT);
    #else
    while (!pbsys_main_program_start_is_requested()) {
        // Keep requesting advertising so that failed or timed-out attempts
        // are retried. pbdrv_bluetooth_start_advertising() is a no-op if
        // already advertising or if advertising state is confirmed.
        #if PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE
        if (pbio_os_timer_is_expired(&auto_advertise_timer)) {
            if (!pbdrv_bluetooth_host_is_connected()) {
                pbdrv_bluetooth_start_advertising(true);
            }
            pbio_os_timer_set(&auto_advertise_timer, PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE_PERIOD_MS);
        }
        #endif

        if (pbsys_status_test(PBIO_PYBRICKS_STATUS_SHUTDOWN_REQUEST)) {
            return PBIO_ERROR_CANCELED;
        }
        pbio_os_run_processes_and_wait_for_event();
    }
    return PBIO_SUCCESS;
    #endif
}

#endif // PBSYS_CONFIG_HMI_NONE
