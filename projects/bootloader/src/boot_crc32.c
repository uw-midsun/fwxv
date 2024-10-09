#include "boot_crc32.h"
#include "stm32f10x.h"
#include "stm32f10x_crc.h"

BootloaderError boot_crc_init(void) {
  RCC->AHBENR |= RCC_AHBENR_CRCEN;

  CRC_ResetDR();

  return BOOTLOADER_ERROR_NONE;
}

uint32_t boot_crc_calculate(const uint32_t *buffer, size_t buffer_len) {
    CRC_ResetDR();

    for (size_t i = 0; i < buffer_len; i++) {
        CRC_CalcCRC(buffer[i]);
    }

    return ~CRC_GetCRC();
}

void boot_align_to_32bit_words(uint8_t *buffer, size_t *buffer_len) {
    uint8_t padding = (4 - (*buffer_len % 4)) % 4;
    memset(buffer + *buffer_len, 0, padding);
    *buffer_len += padding;
}
