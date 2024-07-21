#pragma once
#include "can_datagram.h"
#include "can.h"
#include "global.h"
#include "boot_flash.h"

typedef uintptr_t bootloader_ptr_t;


typedef struct {
  uint8_t dgram_type;
  uint16_t node_ids;
  uint16_t data_len;
  uint8_t *data;
} BootloaderRxPacket;

typedef struct {
    // INCOMPLETE
} BootloaderConfig;

typedef enum {
  /// @brief 
  DATAGRAM_START = 0,
  /// @brief 
  DATAGRAM_DATA,
  /// @brief 
  DATAGRAM_JUMP,
} BootloaderTypeID;

//STATE MACHINE
typedef enum {
  /// @brief datagram starts as uninitialized state
  BOOTLOADER_UNINITIALIZED = 0,
  /// @brief datagram is ready to either jump or flash applications
  BOOTLOADER_IDLE, 
  /// @brief datagram is processing a start message
  BOOTLOADER_START, 
  /// @brief datagram is current receiving the datagram streaming data
  BOOTLOADER_DATA_RECEIVE,
  /// @brief datagram is jumping to the application identified in the message
  BOOTLOADER_JUMP_APP,
  /// @brief datagram encountered an error
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

typedef struct {
    BootloaderStates state;
    BootloaderError error;
    bool first_byte_received;
    uint32_t bytes_received;
    uint32_t binary_size;
} BootloaderStateData;
