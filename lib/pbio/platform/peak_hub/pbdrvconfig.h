// SPDX-License-Identifier: MIT

#define PBDRV_CONFIG_ADC                            (0)

#define PBDRV_CONFIG_BATTERY                        (0)

#define PBDRV_CONFIG_BLUETOOTH                      (0)
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK              (0)
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32        (0)
#define PBDRV_CONFIG_BLUETOOTH_PEAK_ENABLE_STUBS    (1)
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_NUM_LE_HOSTS (1)
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_HUB_KIND     LWP3_HUB_KIND_TECHNIC_LARGE
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_HUB_VARIANT_ADDR 0
#define PBDRV_CONFIG_BLUETOOTH_NUM_CLASSIC_CONNECTIONS (0)
#define PBDRV_CONFIG_BLUETOOTH_NUM_PERIPHERALS      (1)
#define PBDRV_CONFIG_BLUETOOTH_MAX_MTU_SIZE         (247)

// PeakHub Bluetooth coprocessor preparation (feature-gated, disabled by default).
#define PBDRV_CONFIG_BLUETOOTH_PEAK_PREP            (1)
#define PBDRV_CONFIG_BLUETOOTH_PEAK_UART_INSTANCE   LPUART1
#define PBDRV_CONFIG_BLUETOOTH_PEAK_UART_IRQ        LPUART1_IRQn
#define PBDRV_CONFIG_BLUETOOTH_PEAK_UART_BAUD       1000000
#define PBDRV_CONFIG_BLUETOOTH_PEAK_UART_TX_PIN     6  // PB6 (smoke-test mapping)
#define PBDRV_CONFIG_BLUETOOTH_PEAK_UART_RX_PIN     7  // PB7 (smoke-test mapping)
#define PBDRV_CONFIG_BLUETOOTH_PEAK_HCI_PROBE       (0)
#define PBDRV_CONFIG_BLUETOOTH_PEAK_FLOW_PROBE      (1)
// Conservative baseline for ESP32-S3 H:4 bring-up over jumper wiring.
// Raise after stability is confirmed with current hardware path.
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_INIT 57600
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_BAUDRATE_MAIN 57600
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_FLOWCONTROL 0
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_STM32_TELEMETRY 1
#define PBDRV_CONFIG_BLUETOOTH_ADVERTISE_TIMEOUT_RESET_THRESHOLD 0
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_AUTO_RESTART_ADVERTISING_ON_DISCONNECT 1
// Disable immediate path: it sets advertising state optimistically before HCI
// command-complete is confirmed, which prevents all retry attempts on failure.
// Use the async advertising_or_scan_func path instead, which awaits confirmation.
#define PBDRV_CONFIG_BLUETOOTH_BTSTACK_IMMEDIATE_READVERTISE_ON_DISCONNECT 0
// Reserved placeholder GPIOs for future BT flow control wiring.
#define PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PORT        GPIOD
#define PBDRV_CONFIG_BLUETOOTH_PEAK_RTS_PIN         8
#define PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PORT        GPIOD
#define PBDRV_CONFIG_BLUETOOTH_PEAK_CTS_PIN         9
// Set to a board-specific GPIO when hardware is finalized.
#define PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PORT      GPIOB
#define PBDRV_CONFIG_BLUETOOTH_PEAK_RESET_PIN       0xFF

#define PBDRV_CONFIG_BLOCK_DEVICE                   (1)
#define PBDRV_CONFIG_BLOCK_DEVICE_RAM_SIZE          (160 * 1024)
#define PBDRV_CONFIG_BLOCK_DEVICE_FLASH_STM32       (1)
#define PBDRV_CONFIG_BLOCK_DEVICE_FLASH_STM32_SIZE  (128 * 1024)

#define PBDRV_CONFIG_BUTTON                         (0)

#define PBDRV_CONFIG_CLOCK                          (1)
#define PBDRV_CONFIG_CLOCK_STM32                    (1)
#define PBDRV_CONFIG_CLOCK_STM32_HEARTBEAT_LED      (1)

#define PBDRV_CONFIG_COUNTER                        (0)

#define PBDRV_CONFIG_GPIO                           (0)

#define PBDRV_CONFIG_IOPORT                         (1)
#define PBDRV_CONFIG_IOPORT_HAS_UART                (1)
#define PBDRV_CONFIG_IOPORT_NUM_DEV                 (1)

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
#define PBDRV_CONFIG_USB_PROD_STR                   u"PeakHub + Pybricks"
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
