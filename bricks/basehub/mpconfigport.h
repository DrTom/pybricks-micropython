// SPDX-License-Identifier: MIT

#include "stm32h563xx.h"

#define MICROPY_HW_BOARD_NAME                   "BaseHub (STM32H5 prototype)"
#define MICROPY_HW_MCU_NAME                     "STM32H563"

#define PYBRICKS_HUB_NAME                       "basehub"
#define PYBRICKS_HUB_CLASS_NAME                 (MP_QSTR_BaseHub)

#define PYBRICKS_PY_COMMON                      (1)
#define PYBRICKS_PY_COMMON_BLE                  (0)
#define PYBRICKS_PY_COMMON_CHARGER              (0)
#define PYBRICKS_PY_COMMON_COLOR_LIGHT          (0)
#define PYBRICKS_PY_COMMON_CONTROL              (0)
#define PYBRICKS_PY_COMMON_IMU                  (0)
#define PYBRICKS_PY_COMMON_KEYPAD               (0)
#define PYBRICKS_PY_COMMON_LIGHT_ARRAY          (0)
#define PYBRICKS_PY_COMMON_LIGHT_MATRIX         (0)
#define PYBRICKS_PY_COMMON_LOGGER               (1)
#define PYBRICKS_PY_COMMON_MOTOR_MODEL          (0)
#define PYBRICKS_PY_COMMON_MOTORS               (0)
#define PYBRICKS_PY_COMMON_SPEAKER              (0)
#define PYBRICKS_PY_COMMON_SYSTEM               (1)
#define PYBRICKS_PY_EV3DEVICES                  (0)
#define PYBRICKS_PY_EXPERIMENTAL                (0)
#define PYBRICKS_PY_HUBS                        (0)
#define PYBRICKS_PY_IODEVICES                   (0)
#define PYBRICKS_PY_NXTDEVICES                  (0)
#define PYBRICKS_PY_PARAMETERS                  (1)
#define PYBRICKS_PY_PARAMETERS_BUTTON           (0)
#define PYBRICKS_PY_PARAMETERS_ICON             (0)
#define PYBRICKS_PY_PARAMETERS_IMAGE            (0)
#define PYBRICKS_PY_DEVICES                     (0)
#define PYBRICKS_PY_PUPDEVICES                  (0)
#define PYBRICKS_PY_ROBOTICS                    (0)
#define PYBRICKS_PY_TOOLS                       (1)
#define PYBRICKS_PY_TOOLS_HUB_MENU              (0)
#define PYBRICKS_PY_TOOLS_APP_DATA              (0)

#define PYBRICKS_OPT_COMPILER                   (1)
#define PYBRICKS_OPT_USE_STACK_END_AS_TOP       (1)
#define PYBRICKS_OPT_RAW_REPL                   (1)
#define PYBRICKS_OPT_FLOAT                      (1)
#define PYBRICKS_OPT_TERSE_ERR                  (0)
#define PYBRICKS_OPT_EXTRA_LEVEL1               (0)
#define PYBRICKS_OPT_EXTRA_LEVEL2               (0)
#define PYBRICKS_OPT_CUSTOM_IMPORT              (0)
#define PYBRICKS_OPT_NATIVE_MOD                 (0)

#include "../_common/mpconfigport.h"
