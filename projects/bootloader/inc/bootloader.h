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
  BOOTLOADER_FAULT,
  /// @brief Bootloader is ready to start receiving data
  BOOTLOADER_PING,
} BootloaderStates;

typedef enum {
  /// @brief Bootloader should start pinging MCU's
  BOOTLOADER_PING_NODES = 0,
  /// @brief Bootloader ping should do branch stuff
  BOOTLOADER_PING_BRANCH,
  /// @brief Bootloader ping should do group stuff
  BOOTLOADER_PING_PROJECT,
} BootloaderPingStates;

typedef struct {
  uintptr_t application_start;
  uintptr_t current_address;
  uint32_t bytes_written;
  uint32_t data_size;
  uint32_t packet_crc32;
  uint16_t expected_sequence_number;
  uint16_t buffer_index;
  BootloaderPingStates ping_type;
  uint8_t ping_data_len;

  BootloaderStates state;
  BootloaderError error;
  uint16_t target_nodes;
  bool first_byte_received;
  bool first_ping_received;
} BootloaderStateData;

BootloaderError bootloader_init(uint8_t flash_buffer[BOOTLOADER_PAGE_BYTES]); //temporary input to the buffer array
BootloaderError bootloader_run(Boot_CanMessage *msg);
BootloaderError bootloader_jump_app(void);
