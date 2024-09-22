#include "crc32.h"  // Include the header file for CRC32 function declarations

#include "log.h"  // Include the logging utility, possibly used for debugging or status messages
#include "misc.h"
#include "stm32f10x.h"  // Include the STM32F1 series specific header file for hardware definitions
#include "stm32f10x_crc.h"  // Include the CRC library header

// Initialization function for CRC
StatusCode crc_init(void) {
  // Enable the CRC peripheral clock
  RCC->AHBENR |= RCC_AHBENR_CRCEN;

  // Reset CRC Data Register
  CRC_ResetDR();

  return STATUS_CODE_OK;  // Return OK status
}

// Function to calculate CRC32 for a given buffer, with an option to append to an existing CRC
uint32_t crc_calculate(const uint32_t *buffer, size_t buffer_len) {
  CRC_ResetDR();  // Reset the CRC Data Register

  // Process each byte in the buffer
  for (size_t i = 0; i < buffer_len; i++) {
    CRC_CalcCRC(buffer[i]);  // Calculate CRC for each byte
  }

  return ~CRC_GetCRC();  // Return the inverted final CRC value
}
