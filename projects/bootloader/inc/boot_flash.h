#pragma once
#include <stddef.h>
#include <stdint.h>

#include "stm32f10x.h"
#include "global.h"

#define NUM_FLASH_PAGES 128

#define BOOTLOADER_ADDR_TO_PAGE(addr) \
  (((uintptr_t)(addr) - (uintptr_t)APP_START_ADDRESS) / BOOTLOADER_PAGE_BYTES)

#define BOOTLOADER_PAGE_TO_ADDR(page) ((uintptr_t)(page)*BOOTLOADER_PAGE_BYTES + (uintptr_t)APP_START_ADDRESS)
#define BOOTLOADER_WRITE_BYTES 4
#define BOOTLOADER_PAGE_BYTES 0x400

BootloaderError boot_flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len);

BootloaderError boot_flash_erase(uint8_t page);

