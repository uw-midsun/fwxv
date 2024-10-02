#pragma once

#include "boot_can.h"
#include "boot_flash.h"
#include "bootloader_mcu.h"
#include "can_datagram.h"

// STATE MACHINE
typedef enum {
  /// @brief Bootloader starts unitialized
  BOOTLOADER_UNINITIALIZED = 0,
  /// @brief Bootloader is initialized and waiting for messages
  BOOTLOADER_IDLE,
  /// @brief Bootloader is processing a start message
  BOOTLOADER_START,
  /// @brief Bootloader is waiting for the first byte to be sent (CAN_ARBITRATION_SEQUENCING_ID)
  BOOTLOADER_DATA_READY,
  /// @brief Bootloader is receiving streamed data and flashing it immediately
  /// (CAN_ARBITRATION_FLASH_ID)
  BOOTLOADER_DATA_RECEIVE,
  /// @brief Bootloader is prompted to jump to application defined by APP_START_ADDRESS
  BOOTLOADER_JUMP_APP,
  /// @brief Bootloader is in fault state
  BOOTLOADER_FAULT
} BootloaderStates;

typedef struct {
  uintptr_t application_start;
  uintptr_t current_address;
  uint32_t bytes_written;
  uint32_t binary_size;
  uint32_t packet_crc32;
  uint16_t expected_sequence_number;
  uint16_t buffer_index;

  BootloaderStates state;
  BootloaderError error;
  uint16_t target_nodes;
  bool first_byte_received;

} BootloaderStateData;

BootloaderError bootloader_init(void);
BootloaderError bootloader_run(Boot_CanMessage *msg);
BootloaderError bootloader_jump_app(void);
