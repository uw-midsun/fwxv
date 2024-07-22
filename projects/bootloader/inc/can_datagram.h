#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "can.h"
#include "global.h"

#define DGRAM_MAX_MSG_SIZE 8

#define NODE_IDS_SIZE_BYTES 2
#define DATA_LEN_SIZE_BYTES 2

#define CAN_ARBITRATION_START_ID 30
#define CAN_ARBITRATION_START_FLASH_ID 31
#define CAN_ARBITRATION_FLASH_ID 32
#define CAN_ARBITRATION_JUMP_ID 33
#define CAN_ARBITRATION_FAULT_ID 34

#define CAN_DATAGRAM_SOF 0xAA
#define CAN_DATAGRAM_EOF 0xBB

#define CAN_BITRATE 500000

#define MIN_BYTEARRY_SIZE 4

#define NODE_IDS_OFFSET 0
#define DATA_SIZE_OFFSET 2

#define LITTLE_ENDIANIZE(value, outbuf)                 \
  for (size_t byte = 0; byte < sizeof(value); byte++) { \
    outbuf[byte] = value >> (8 * byte) & 0xFF;          \
  }

typedef struct {
  uint8_t datagram_type_id;  // Set as arbitration id
  union {
    struct {
      uint16_t node_ids;
      uint16_t data_len;
    } start;
    struct {
      uint8_t *binary_data;
    } data;
    struct {
      // TO BE DEFINED
    } error;
  } payload;
} can_datagram_t;

can_datagram_t unpack_datagram(CanMessage *msg, bool first_byte_received);
