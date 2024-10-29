/*
CRC32 for calculating crc calculation on the STM33F10x Board in ARM
Another crc32.c file in x86 that does not use HAL Libraries and used an algorithm to calculated crc checksum
*/


#include "crc32.h" 
#include "log.h"  
#include "misc.h"
#include "stm32f10x.h"  
#include "stm32f10x_crc.h" 

// Initialization function for CRC
StatusCode crc_init(void) {
  // Enable the CRC peripheral clock
  RCC->AHBENR |= RCC_AHBENR_CRCEN;

  // Reset CRC Data Register
  CRC_ResetDR();

  return STATUS_CODE_OK;  // Return OK status
}

// Function to calculate CRC32 for a given buffer
uint32_t crc_calculate(const uint32_t *buffer, size_t buffer_len) {
  CRC_ResetDR();  // Reset the CRC Data Register

  // Process each byte in the buffer
  for (size_t i = 0; i < buffer_len; i++) {
    CRC_CalcCRC(buffer[i]);  // Calculate CRC for each byte
  }

  return ~CRC_GetCRC();  // Return the inverted final CRC value
  // Convention of CRC standards to return the inverted value
  // Single bit errors are easier to detect 
}
