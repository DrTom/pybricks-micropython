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
#include <pbsys/main.h>
#include <pbsys/status.h>

#ifndef PBSYS_CONFIG_HMI_NONE_STARTS_REPL
#define PBSYS_CONFIG_HMI_NONE_STARTS_REPL (1)
#endif

#ifndef PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE
#define PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE (0)
#endif

void pbsys_hmi_init(void) {
}

void pbsys_hmi_deinit(void) {
}

void pbsys_hmi_stop_animation(void) {
}

pbio_error_t pbsys_hmi_await_program_selection(void) {

    do {
        #if PBSYS_CONFIG_HMI_NONE_AUTO_ADVERTISE
        pbdrv_bluetooth_start_advertising(true);
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
        pbdrv_bluetooth_start_advertising(true);
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
