/* SPDX-License-Identifier: MIT */

MEMORY
{
    FLASH_FIRMWARE (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM (xrw)           : ORIGIN = 0x24000000, LENGTH = 512K
}

_minimal_stack_size = 8K;

/* Used when PBSYS_CONFIG_STORAGE is disabled. */
pbsys_storage_heap_start = ORIGIN(RAM);
pbsys_storage_heap_end = ORIGIN(RAM) + LENGTH(RAM);
