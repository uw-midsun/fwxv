#pragma once
#include "boot_flash.h"
#include "boot_can.h"
#include "can_datagram.h"
#include "global.h"

typedef struct {
  // INCOMPLETE
} BootloaderConfig;

// STATE MACHINE
typedef enum {
  /// @brief Bootloader starts unitialized
  BOOTLOADER_UNINITIALIZED = 0,
  /// @brief Bootloader is initialized and waiting for messages
  BOOTLOADER_IDLE,
  /// @brief Bootloader is processing a start message
  BOOTLOADER_START,
  /// @brief Bootloader is waiting for the first byte to be sent (CAN_ARBITRATION_START_FLASH_ID)
  BOOTLOADER_DATA_READY,
  /// @brief Bootloader is receiving streamed data and flashing it immediately (CAN_ARBITRATION_FLASH_ID)
  BOOTLOADER_DATA_RECEIVE,
  /// @brief Bootloader is prompted to jump to application defined by APP_START_ADDRESS
  BOOTLOADER_JUMP_APP,
  /// @brief Bootloader is in fault state
  BOOTLOADER_FAULT
} BootloaderStates;

/**
 * @brief gets the current state machine state of the bootloader
 * @return Bootloader's current state type
 */
BootloaderStates bootloader_get_state(void);

/**
 * @brief get the most recent error from the state machine
 * @return returns BOOTLOADER_ERROR_NONE when no errors occured
 */
BootloaderError bootloader_get_err(void);

BootloaderError bootloader_init(void);

BootloaderError bootloader_run(Boot_CanMessage *msg);

typedef struct {
  BootloaderStates state;
  BootloaderError error;
  bool first_byte_received;

  uintptr_t current_address;
  uint32_t bytes_written;
  uint32_t binary_size;
} BootloaderStateData;
