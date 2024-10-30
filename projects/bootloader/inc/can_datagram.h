#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "boot_can.h"
#include "bootloader_mcu.h"

#define DGRAM_MAX_MSG_SIZE 8
#define CAN_BITRATE 500000

typedef enum {
  CAN_ARBITRATION_SEQUENCING_ID = 30,
  CAN_ARBITRATION_FLASH_ID,
  CAN_ARBITRATION_JUMP_ID,
  CAN_ARBITRATION_ACK_ID,
  CAN_ARBITRATION_START_ID,
  CAN_ARBITRATION_JUMP_BOOTLOADER,
  CAN_ARBITRATION_METADATA,
  CAN_ARBITRATION_PING,
  CAN_PING_NODE_ID,
  CAN_PING_PROJECT,
  CAN_PING_BRANCH,
} BootloaderCanID;

typedef struct {
  uint8_t datagram_type_id;
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
      uint8_t ack_status;
      uint16_t bootloader_error;
    } ack;
    struct {
      //metadata
      uint8_t metadata;
      //project / Git branch
      uint8_t* branchproject;
      //node_id
      uint8_t node_id;
      //isGitorProject
      uint8_t isBranch;
      
    } ping;
  } payload;
} BootloaderDatagram_t;

BootloaderDatagram_t unpack_datagram(Boot_CanMessage *msg, uint16_t *target_nodes);
void send_ack_datagram(bool ack, BootloaderError error);
