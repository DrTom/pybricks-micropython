# PeakHub BTstack Soak Checklist

Use this checklist for the current conservative baseline:

- UART: 115200 baud
- H:4 flow control: disabled on both sides
- ESP32-S3 `hci_uart`: `CONFIG_BT_HCI_UART_FLOW_CONTROL=n`

## Setup

1. Flash latest PeakHub firmware.
2. Boot ESP32-S3 HCI UART firmware.
3. Connect `PB6 -> ESP RX`, `PB7 <- ESP TX`, and GND.
4. Confirm USB + `pybricksdev` connectivity still works.

## Snapshot Fields

Watch this symbol in debugger:

- `pbdrv_btstack_stm32_telemetry_snapshot`

Most useful fields:

- `seq`
- `init_count`
- `hci_event_count`
- `hci_last_event_code`
- `hci_last_status`
- `uart_send_bytes`
- `uart_recv_bytes`
- `uart_tx_irq_count`
- `uart_rx_irq_count`

## Test Procedure

1. Record a baseline snapshot at boot (`T0`).
2. Run for 10-15 minutes.
3. During the run, do at least 10 USB upload/run cycles.
4. Record a second snapshot at end (`T1`).

## Pass/Fail Gates

Pass if all are true:

- `seq` changes over time (snapshot updates).
- `init_count` does not increase after startup (no BT re-init loops).
- `hci_event_count` is non-zero by `T1`.
- `uart_recv_bytes` and `uart_rx_irq_count` are non-zero by `T1`.
- USB upload/run remains stable during the full run.

Warning (investigate, but not auto-fail yet):

- `hci_last_status` is non-zero at `T1`.
- `hci_unknown_count` keeps rising rapidly while no useful HCI progress occurs.

Fail if any are true:

- Hard fault, reboot loop, or frozen firmware.
- `init_count` increments repeatedly during soak.
- `uart_recv_bytes == 0` at `T1` (TX-only behavior).
- USB upload/run regression appears.

## Optional Step-Up Plan

Only after a clean pass at 115200:

1. Try 230400 (flow control still off).
2. Repeat the same soak checklist.
3. Keep the highest rate that passes twice in a row.
