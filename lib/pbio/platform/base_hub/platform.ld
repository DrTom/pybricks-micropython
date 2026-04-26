/* SPDX-License-Identifier: MIT */

MEMORY
{
    FLASH_FIRMWARE (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM (xrw)           : ORIGIN = 0x20000000, LENGTH = 640K
}

_minimal_stack_size = 16K;

/* Used when PBSYS_CONFIG_STORAGE is disabled. */
pbsys_storage_heap_start = _ebss;
pbsys_storage_heap_end = pbdrv_stack_end - _minimal_stack_size;
