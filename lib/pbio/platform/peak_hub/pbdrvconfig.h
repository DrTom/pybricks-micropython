// SPDX-License-Identifier: MIT

#define PBDRV_CONFIG_ADC                            (0)

#define PBDRV_CONFIG_BATTERY                        (0)

#define PBDRV_CONFIG_BLUETOOTH                      (0)

#define PBDRV_CONFIG_BLOCK_DEVICE                   (0)

#define PBDRV_CONFIG_BUTTON                         (0)

#define PBDRV_CONFIG_CLOCK                          (1)
#define PBDRV_CONFIG_CLOCK_STM32                    (1)
#define PBDRV_CONFIG_CLOCK_STM32_HEARTBEAT_LED      (1)

#define PBDRV_CONFIG_COUNTER                        (0)

#define PBDRV_CONFIG_GPIO                           (0)

#define PBDRV_CONFIG_IOPORT                         (0)
#define PBDRV_CONFIG_IOPORT_HAS_UART                (1)

#define PBDRV_CONFIG_LED                            (0)

#define PBDRV_CONFIG_MOTOR_DRIVER                   (0)

#define PBDRV_CONFIG_PWM                            (0)

#define PBDRV_CONFIG_RANDOM                         (0)

#define PBDRV_CONFIG_RESET                          (0)
#define PBDRV_CONFIG_RESET_STM32                    (0)
#define PBDRV_CONFIG_RESET_STM32_HAS_BLE_BOOTLOADER (0)

#define PBDRV_CONFIG_SOUND                          (0)

#define PBDRV_CONFIG_UART                           (1)
#define PBDRV_CONFIG_UART_DEBUG_FIRST_PORT          (0)
#define PBDRV_CONFIG_UART_STM32H7_LL_DMA            (1)
#define PBDRV_CONFIG_UART_STM32H7_LL_DMA_NUM_UART   (8)
#define PBDRV_CONFIG_UART_STM32H7_LL_IRQ            (0)
#define PBDRV_CONFIG_UART_STM32H7_LL_IRQ_NUM_UART   (2)

#define PBDRV_CONFIG_USB                            (1)
#define PBDRV_CONFIG_USB_MAX_PACKET_SIZE            (64)
#define PBDRV_CONFIG_USB_NUM_BUFFERED_PACKETS       (2)
#define PBDRV_CONFIG_USB_VID                        LEGO_USB_VID
#define PBDRV_CONFIG_USB_PID                        0xFFFF
#define PBDRV_CONFIG_USB_MFG_STR                    LEGO_USB_MFG_STR
#define PBDRV_CONFIG_USB_PROD_STR                   u"Hub Peak + Pybricks"
#define PBDRV_CONFIG_USB_STM32H7                    (1)
#define PBDRV_CONFIG_USB_CHARGE_ONLY                (0)

#define PBDRV_CONFIG_STACK                          (1)
#define PBDRV_CONFIG_STACK_EMBEDDED                 (1)

#define PBDRV_CONFIG_WATCHDOG                       (0)

#define PBDRV_CONFIG_HAS_PORT_A                     (1)
#define PBDRV_CONFIG_HAS_PORT_B                     (1)
#define PBDRV_CONFIG_HAS_PORT_VCC_CONTROL           (0)

#define PBDRV_CONFIG_SYS_CLOCK_RATE                 64000000
#define PBDRV_CONFIG_INIT_ENABLE_INTERRUPTS_ARM     (1)
