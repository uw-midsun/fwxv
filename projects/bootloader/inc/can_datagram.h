#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "boot_can.h"
#include "bootloader_mcu.h"

#define DGRAM_MAX_MSG_SIZE 8

#define NODE_IDS_SIZE_BYTES 2
#define DATA_LEN_SIZE_BYTES 2

#define CAN_ARBITRATION_START_ID 30
#define CAN_ARBITRATION_SEQUENCING_ID 31
#define CAN_ARBITRATION_FLASH_ID 32
#define CAN_ARBITRATION_JUMP_ID 33
#define CAN_ARBITRATION_ACK_ID 34

#define CAN_BITRATE 500000

#define MIN_BYTEARRY_SIZE 4

typedef struct {
  uint8_t datagram_type_id;  // Set as arbitration id
  union {
    struct {
      uint16_t node_ids;
      uint32_t data_len;
    } start;
    struct {
      uint16_t sequence_num;
      uint32_t crc32;
    } sequencing;
    struct {
      uint8_t *binary_data;
    } data;
    struct {
      uint16_t node_ids;
    } jump_app;
    struct {
      // TO BE DEFINED
    } error;
  } payload;
} can_datagram_t;

can_datagram_t unpack_datagram(Boot_CanMessage *msg, uint16_t *target_nodes);
void send_ack_datagram(bool ack, BootloaderError error);
