#pragma once

// The addresses of these values are defined in the bootloader linker script (memory_spaces.ld).
// If there's an undefined reference linker error from here, you're using the wrong linker script!
extern uint32_t _application_start;
extern uint32_t _application_size;
extern uint32_t _ram_start;
extern uint32_t _ram_size;
extern uint32_t _vector_table_size;

#define APP_START_ADDRESS ((uint32_t)&_application_start)
#define BOOTLOADER_APPLICATION_SIZE ((size_t)_application_size)
#define RAM_START_ADDRESS (_ram_start)
#define BOOTLOADER_RAM_SIZE ((size_t)_ram_size)
#define BOOTLOADER_VECTOR_TABLE_SIZE ((size_t)_vector_table_size)

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
  BOOTLOADER_SEQUENCE_ERROR
} BootloaderError;
