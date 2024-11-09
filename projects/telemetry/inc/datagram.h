#pragma once

#include "can.h"

#pragma pack (push, 1)
typedef struct {
  // Initialize start_frame as 0xAA
  uint8_t start_frame;
  uint32_t id;
  size_t dlc;

  // Only use dlc number of bytes in data, plus 1 byte for end_of_frame
  uint8_t data[9];


  // union {
  //   uint64_t data_u64;
  //   uint32_t data_u32[2];
  //   uint16_t data_u16[4];
  //   uint8_t data_u8[8];
  // } data;

  // Initialize end_of_frame as 0xBB
} Datagram;
#pragma pack(pop)

size_t decode_can_message(Datagram *datagram, CanMessage *msg);
