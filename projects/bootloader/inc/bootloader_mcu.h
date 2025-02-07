#pragma once

// The addresses of these values are defined in the bootloader linker script (memory_spaces.ld).
// If there's an undefined reference linker error from here, you're using the wrong linker script!
extern uint32_t _application_start;
extern uint32_t _application_size;
extern uint32_t _ram_start;
extern uint32_t _ram_size;
extern uint32_t _vector_table_size;
extern uint32_t _flash_start;

#define APP_START_ADDRESS ((uint32_t) &_application_start)
#define APPLICATION_SIZE ((size_t)&_application_size)
#define RAM_START_ADDRESS (_ram_start)
#define BOOTLOADER_RAM_SIZE ((size_t)_ram_size)
#define BOOTLOADER_VECTOR_TABLE_SIZE ((size_t)_vector_table_size)
#define FLASH_START_ADDRESS ((uint32_t) &_flash_start)

#define NACK  0
#define ACK   1

// ERROR DATA
typedef enum {
  /// @brief no data is found
  BOOTLOADER_ERROR_NONE = 0,
  /// @brief more data recieved than anticipated
  BOOTLOADER_OVERSIZE,
  /// @brief data is not 4byte aligned
  BOOTLOADER_DATA_NOT_ALIGNED,
  /// @brief authenication is not valid
  BOOTLOADER_INVALID_ID,
  /// @brief the can bus encountered an error
  BOOLOADER_TRANSMISSION_E,
  /// @brief unidentified internal controller error
  BOOTLOADER_INTERNAL_ERR,
  /// @brief agruements given doesn't match the bootloader's requirements
  BOOTLOADER_INVALID_ARGS,
  /// @brief Initialization failed
  BOOTLOADER_ERROR_UNINITIALIZED,
  /// @brief flash_erase or flash_write error
  BOOTLOADER_FLASH_ERR,
  /// @brief Bootloader flash write failed because data is not aligned
  BOOTLOADER_FLASH_WRITE_NOT_ALIGNED,
  /// @brief Bootloader flash write failed because address out of bounds
  BOOTLOADER_FLASH_WRITE_OUT_OF_BOUNDS,
  /// @brief Bootloader flash read failed
  BOOTLOADER_FLASH_READ_FAILED,
  /// @brief flash buffer has overflowed (more than 1024 bytes)
  BOOTLOADER_BUFFER_OVERFLOW,
  /// @brief Bootloader CAN init error
  BOOTLOADER_CAN_INIT_ERR,
  /// @brief Bootloader attempted to write/read to incorrect address
  BOOTLOADER_INVALID_ADDRESS,
  /// @brief Bootloader has detected a crc mismatch before writing to flash
  BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE,
  /// @brief Bootloader has detected a crc mismatch after reading the new flash memory
  BOOTLOADER_CRC_MISMATCH_AFTER_WRITE,
  /// @brief Bootloader received an incorrect sequence number
  BOOTLOADER_SEQUENCE_ERROR,
  /// @brief Bootloader time out
  BOOTLOADER_TIMEOUT,
  /// @brief Bootloader flash memory verification failed. Everything is still erased
  BOOTLOADER_FLASH_MEMORY_VERIFY_FAILED,
  /// @brief Bootloader ping timeout
  BOOTLOADER_PING_TIMEOUT
} BootloaderError;
