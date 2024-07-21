#include "boot_flash.h"

BootloaderError boot_flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (address < APP_START_ADDRESS || (address + buffer_len) > APP_END_ADDRESS) {
    return BOOTLOADER_FLASH_ERR;
  } else if (buffer_len % BOOTLOADER_WRITE_BYTES != 0 || address % BOOTLOADER_WRITE_BYTES != 0) {
    return BOOTLOADER_FLASH_ERR;
  }

  uint32_t *data = (uint32_t *)buffer;
  size_t data_len = buffer_len / BOOTLOADER_WRITE_BYTES;

  FLASH_Unlock();
  FLASH_Status status = FLASH_COMPLETE;
  for (size_t i = 0; status == FLASH_COMPLETE && i < data_len; i++) {
    status = FLASH_ProgramWord(address + (i * BOOTLOADER_WRITE_BYTES), data[i]);
    while (FLASH->SR & FLASH_SR_BSY)
      ;
  }
  FLASH_Lock();

  if (status != FLASH_COMPLETE) {
    return BOOTLOADER_FLASH_ERR;
  }
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_erase(uint8_t page) {
  if (page > NUM_FLASH_PAGES) {
    return BOOTLOADER_FLASH_ERR;
  }
  FLASH_Unlock();
  FLASH_Status status = FLASH_ErasePage(BOOTLOADER_PAGE_TO_ADDR(page));
  FLASH_Lock();

  if (status != FLASH_COMPLETE) {
    return BOOTLOADER_FLASH_ERR;
  }
  return BOOTLOADER_ERROR_NONE;
}