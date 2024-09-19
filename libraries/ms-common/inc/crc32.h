#pragma once

#include "FreeRTOS.h"
#include "status.h"
#include <stddef.h>
#include <stdint.h>

// Initialize CRC calculation (no use for STM32F1)
StatusCode crc32_init(void);

// Calculate CRC32 for an entire buffer
uint32_t crc32_arr(const uint8_t *buffer, size_t buffer_len);

// Incremental CRC32 calculation
uint32_t crc32_append_arr(const uint8_t *buffer, size_t buffer_len, uint32_t initial_crc);