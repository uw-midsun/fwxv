#pragma once

#define NODE_ID 0 // Maybe write this into flash memory? Not determined currently
#define APP_START_ADDRESS 0x8000400 // Defined in linkerscript
#define APP_END_ADDRESS 0x08010000 // Defined in linkerscript

//ERROR DATA
typedef enum {
  /// @brief no data is found
  BOOTLOADER_ERROR_NONE = 0,
  /// @brief more data recieved than anticipated
  BOOTLOADER_OVERSIZE,
  /// @brief authenication is not valid 
  BOOTLOADER_INVALID_ID,
  /// @brief the can bus encountered an error
  BOOLOADER_TRANSMISSION_E,
  /// @brief unidentified internal controller error
  BOOTLOADER_INTERNAL_ERR,
  /// @brief agruements given doesn't match the bootloader's requirements 
  BOOTLOADER_INVALID_ARGS,
  /// @brief flash_erase or flash_write error
  BOOTLOADER_FLASH_ERR,
} BootloaderError;
