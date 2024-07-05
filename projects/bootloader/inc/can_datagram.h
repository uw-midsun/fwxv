#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "status.h"

#define DGRAM_MAX_DATA_SIZE 2048
#define DGRAM_MAX_MSG_SIZE 8

#define DGRAM_TYPE_SIZE_BYTES 1
#define NODE_IDS_SIZE_BYTES 2
#define DATA_LEN_SIZE_BYTES 2


#define CAN_START_ARBITRATION_ID 0b00000010000
#define CAN_START_ID 0b00000000000
#define CAN_ARBITRATION_FLASH_ID 0b00000000001
#define CAN_ARBITRATION_JUMP_ID 0b00000000010

#define CAN_BITRATE 500000

#define MIN_BYTEARRY_SIZE 5

#define DATAGRAM_TYPE_OFFSET 0
#define NODE_IDS_OFFSET 1 
#define DATA_SIZE_OFFSET 3  

#define LITTLE_ENDIANIZE(value, outbuf)                 \
  for (size_t byte = 0; byte < sizeof(value); byte++) { \
    outbuf[byte] = value >> (8 * byte) & 0xFF;          \
  }


typedef uintptr_t bootloader_ptr_t;

typedef struct {
  uint8_t dgram_type;
  uint16_t node_ids;
  uint16_t data_len;
  uint8_t *data;
} BootloaderRxPacket;


typedef struct {


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
  /// @brief datagram received initial first message, ready to recieve binary flashing data
  BOOTLOADER_DATA_READY,
  /// @brief datagram is current receiving the datagram streaming data
  BOOTLOADER_DATA_RECIEVE,
  /// @brief datagram is jumping to the application identified in the message
  BOOTLOADER_JUMP_APP,
  /// @brief datagram encountered an error
  BOOTLOADER_FAULT
} BootloaderStates;

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
  BOOTLOADER_INVALID_ARGS
} BootloaderError;


BootloaderError bootloader_switch_states(const BootloaderStates new_state);
/**
 * @brief updates the state of the datagram 
 * @param  takes in the new datagram state to change to
 * @return returns CanDatagramStates of the updated state
 */

BootloaderStates bootloader_get_state(void);
/**
 * @brief gets the current state machine state of the bootloader
 * @return Bootloader's current state type
 */

BootloaderError bootloader_get_err(void);
/**
 * @brief get the most recent error from the state machine
 * @return returns BOOTLOADER_ERROR_NONE when no errors occured
 */