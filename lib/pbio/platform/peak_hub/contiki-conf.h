// SPDX-License-Identifier: MIT

#ifndef _PBIO_CONF_H_
#define _PBIO_CONF_H_

#include <stdint.h>

#define CCIF
#define CLIF

typedef uint32_t clock_time_t;
#define CLOCK_CONF_SECOND 1000

#define clock_time pbdrv_clock_get_ms
#define clock_usecs pbdrv_clock_get_us

#endif /* _PBIO_CONF_H_ */
