#pragma once

#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "status.h"

// Initialize CRC calculation (no use for STM32F1)
StatusCode crc_init(void);

// Calculate CRC32 for an entire buffer
uint32_t crc_calculate(const uint32_t *buffer, size_t buffer_len);
