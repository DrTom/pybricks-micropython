// SPDX-License-Identifier: MIT
// Copyright (c) 2022 The Pybricks Authors

/**
 * @addtogroup SystemMain System: Main Program
 * @{
 */

#ifndef _PBSYS_MAIN_H_
#define _PBSYS_MAIN_H_

#include <pbio/error.h>

#include <pbsys/config.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * Main application program data information.
 */
typedef struct _pbsys_main_program_t {
    /**
     * Starting address of the user code.
     */
    void *code_start;
    /**
     * Ending address of the user code (up until this, so excluding this address).
     */
    void *code_end;
    /**
     * Starting address of user RAM.
     */
    void *user_ram_start;
    /**
     * Ending address of user RAM (up until this, so excluding this address).
     */
    void *user_ram_end;
    /**
     * User program identifier.
     */
    pbio_pybricks_user_program_id_t id;
    /**
     * Whether a request was made to start the program.
     */
    bool start_requested;
} pbsys_main_program_t;

#if PBSYS_CONFIG_MAIN

pbio_error_t pbsys_main_program_request_start(pbio_pybricks_user_program_id_t id);

/**
 * Validates the program that is being requested to start.
 *
 * @param [in]  program The program that is about to start.
 * @returns     ::PBIO_ERROR_NOT_SUPPORTED if the program is not available.
 *              Otherwise ::PBIO_SUCCESS.
 */
pbio_error_t pbsys_main_program_validate(pbsys_main_program_t *program);

/**
 * Runs the main application program.
 *
 * This should be provided by the application running on top of pbio.
 *
 * @param [in]  program   Program size and data
 */
void pbsys_main_run_program(pbsys_main_program_t *program);

/**
 * Stops (cancels) the main application program.
 *
 * This should be provided by the application running on top of pbio.
 *
 * @param [in]  force_stop  Whether to force stop the program instead of asking
 *                          nicely. This is true when the application must stop
 *                          on shutdown.
 */
void pbsys_main_stop_program(bool force_stop);

/**
 * Handles standard input.
 *
 * This should be provided by the application running on top of pbio.
 *
 * @param [in]  c   the character received
 * @return          *true* if the character was handled and should not be
 *                  placed in the stdin buffer, otherwise *false*.
 */
bool pbsys_main_stdin_event(uint8_t c);

#else // PBSYS_CONFIG_MAIN

static inline pbio_error_t pbsys_main_program_request_start(pbio_pybricks_user_program_id_t id) {
    return PBIO_ERROR_NOT_SUPPORTED;
}

static inline pbio_error_t pbsys_main_program_validate(pbsys_main_program_t *program) {
    return PBIO_ERROR_NOT_SUPPORTED;
}

static inline void pbsys_main_run_program(pbsys_main_program_t *program) {
}

static inline void pbsys_main_stop_program(bool force_stop) {
}

static inline bool pbsys_main_stdin_event(uint8_t c) {
    return false;
}

#endif // PBSYS_CONFIG_MAIN


#endif // _PBSYS_MAIN_H_

/** @} */
