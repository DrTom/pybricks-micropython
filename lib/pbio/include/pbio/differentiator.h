// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2023 The Pybricks Authors

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022-2023 LEGO System A/S

/**
 * @addtogroup Differentiator pbio/differentiator: Differentiation of angle signals.
 *
 * Differentiation of angles to get speed estimates.
 * @{
 */

#ifndef _PBIO_DIFFERENTIATOR_H_
#define _PBIO_DIFFERENTIATOR_H_

#include <stdint.h>

#include <pbio/config.h>
#include <pbio/angle.h>
#include <pbio/control_settings.h>

/**
 * Differentiator of position signal.
 */
typedef struct _pbio_differentiator_t {
    /**
     * Ring buffer of position samples.
     */
    pbio_angle_t history[PBIO_CONFIG_DIFFERENTIATOR_BUFFER_SIZE];
    /**
     * Ring buffer index.
     */
    uint8_t index;
} pbio_differentiator_t;

int32_t pbio_differentiator_update_and_get_speed(pbio_differentiator_t *dif, const pbio_angle_t *angle);

pbio_error_t pbio_differentiator_get_speed(pbio_differentiator_t *dif, uint32_t window, int32_t *speed);

void pbio_differentiator_reset(pbio_differentiator_t *dif, const pbio_angle_t *angle);

#endif // _PBIO_DIFFERENTIATOR_H_

/** @} */
