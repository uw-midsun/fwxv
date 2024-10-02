#include "boot_flash.h"

BootloaderError boot_flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (address < APP_START_ADDRESS) {
    return BOOTLOADER_FLASH_WRITE_OUT_OF_BOUNDS;
  } else if (buffer_len % BOOTLOADER_WRITE_BYTES != 0 || address % BOOTLOADER_WRITE_BYTES != 0) {
    return BOOTLOADER_FLASH_WRITE_NOT_ALIGNED;
  }

  uint32_t *data = (uint32_t *)buffer;
  size_t data_len = buffer_len / BOOTLOADER_WRITE_BYTES;

  FLASH_Unlock();
  // volatile FLASH_Status status = FLASH_COMPLETE;
  for (size_t i = 0; i < data_len; i++) {
    FLASH_ProgramWord(address + (i * BOOTLOADER_WRITE_BYTES), data[i]);
    while (FLASH->SR & FLASH_SR_BSY);  // Wait for the flash operation to complete
  }
  FLASH_Lock();
  if (FLASH->SR & FLASH_SR_EOP) {
    FLASH->SR |= FLASH_SR_EOP;
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_erase(uint8_t page) {
  if (page >= NUM_FLASH_PAGES) {
    return BOOTLOADER_FLASH_ERR;
  }

  FLASH_Unlock();
  FLASH_Status status = FLASH_ErasePage(BOOTLOADER_PAGE_TO_ADDR(page));
  while (FLASH->SR & FLASH_SR_BSY);  // Wait for the flash operation to complete
  FLASH_Lock();

  if (status != FLASH_COMPLETE) {
    return BOOTLOADER_FLASH_ERR;
  }
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_flash_read(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (address < APP_START_ADDRESS) {
    return BOOTLOADER_FLASH_READ_FAILED;
  }

  for (size_t i = 0; i < buffer_len; i++) {
    buffer[i] = *((volatile uint8_t *)(address + i));
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError boot_verify_flash_memory() {
  const uint32_t *flash_pointer = (const uint32_t *)FLASH_START_ADDRESS;
  uint32_t size_in_words = APPLICATION_SIZE / sizeof(uint32_t);

  for (uint32_t i = 0; i < size_in_words; i++) {
    if (flash_pointer[i] != 0xFFFFFFFF) {
      return BOOTLOADER_ERROR_NONE;
    }
  }
  
  return BOOTLOADER_FLASH_ERR;
}
