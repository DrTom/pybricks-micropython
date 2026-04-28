/* SPDX-License-Identifier: MIT */

MEMORY
{
    FLASH_FIRMWARE (rx) : ORIGIN = 0x08000000, LENGTH = 992K
    FLASH_USER_0 (rx)   : ORIGIN = 0x080F8000, LENGTH = 32K
    RAM (xrw)           : ORIGIN = 0x20000000, LENGTH = 640K
}

_minimal_stack_size = 16K;

/* Used when PBSYS_CONFIG_STORAGE is disabled. */
pbsys_storage_heap_start = _ebss;
pbsys_storage_heap_end = pbdrv_stack_end - _minimal_stack_size;

/* Start of persistent storage area in internal flash. */
_pbdrv_block_device_storage_start = ORIGIN(FLASH_USER_0);

/* Portion of storage area checked by checksum complement. */
_pbsys_storage_checked_size = LENGTH(FLASH_USER_0);
