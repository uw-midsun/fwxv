#include "crc32.h"          // Include the header file for CRC32 function declarations
#include "log.h"            // Include the logging utility, possibly used for debugging or status messages
#include "stm32f10x.h"      // Include the STM32F1 series specific header file for hardware definitions (not strictly necessary for this software CRC implementation)

// Define the CRC32 polynomial and initial value
#define CRC32_POLYNOMIAL 0x04C11DB7  // Standard CRC32 polynomial (used in many protocols and formats)
#define CRC32_INIT_VALUE 0xFFFFFFFF  // Initial value used to start the CRC calculation

// Function to calculate CRC32 for a single byte
// This function applies the polynomial division process to a single byte of data
static uint32_t crc32_calculate_byte(uint32_t crc, uint8_t data) {
  crc ^= (uint32_t)data << 24;  // XOR the byte into the top of the 32-bit CRC value

  // Perform the polynomial division one bit at a time
  for (int i = 0; i < 8; i++) {
    if (crc & 0x80000000) {  // If the top bit is set (MSB is 1)
      crc = (crc << 1) ^ CRC32_POLYNOMIAL;  // Shift left and XOR with the polynomial
    } else {
      crc <<= 1;  // Just shift left if the top bit is not set
    }
  }
  return crc;  // Return the updated CRC value after processing the byte
}

// Initialization function for CRC32 (specific to STM32F1)
// Since there's no hardware CRC peripheral on STM32F1, this function does nothing and just returns OK status
StatusCode crc32_init(void) {
  // For STM32F1, there's no hardware CRC peripheral to initialize
  // Simply return OK status
  return STATUS_CODE_OK;
}

// Function to calculate CRC32 for an entire array of bytes
// Calls crc32_append_arr with an initial CRC value of 0
uint32_t crc32_arr(const uint8_t *buffer, size_t buffer_len) {
  return crc32_append_arr(buffer, buffer_len, 0);
}

// Function to calculate CRC32 for a given buffer, with an option to append to an existing CRC
// This allows calculating CRC incrementally if needed
uint32_t crc32_append_arr(const uint8_t *buffer, size_t buffer_len, uint32_t initial_crc) {
  uint32_t crc = ~initial_crc; // Initialize CRC with the inverted initial value (standard practice)

  // Process each byte in the buffer
  for (size_t i = 0; i < buffer_len; i++) {
    crc = crc32_calculate_byte(crc, buffer[i]);  // Update CRC for each byte
  }

  return ~crc; // Return the inverted CRC32 value (final CRC value needs to be inverted)
}
