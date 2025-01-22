#pragma once

#include "can.h"

#pragma pack(push, 1)  // Ensure tight packing of the structure
typedef struct {
  uint8_t start_frame;  // Should always be initialized to 0xAA
  uint32_t id;          // CAN message ID
  size_t dlc;           // Data Length Code: number of bytes in the message payload

  uint8_t data[9];      // Holds the payload; maximum size is 8 bytes + 1 for end_frame
  uint8_t end_frame;    // Should always be initialized to 0xBB
} Datagram;
#pragma pack(pop)

// Function to decode a CAN message into a Datagram
size_t decode_can_message(Datagram *datagram, CanMessage *msg);
