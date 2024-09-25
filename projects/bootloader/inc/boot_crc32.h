#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bootloader.h"

#define BYTES_TO_WORD(bytes)        (bytes / 4)

BootloaderError crc_init(void);
uint32_t crc_calculate(const uint32_t *buffer, size_t buffer_len);
void align_to_32bit_words(uint8_t *buffer, size_t *buffer_len);
