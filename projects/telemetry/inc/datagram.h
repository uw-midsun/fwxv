#pragma once

#include "can.h"

/** @brief  Datagram buffer size */
#define DATAGRAM_BUFFER_SIZE 128U
/** @brief  Start of frame indicator */
#define DATAGRAM_START_FRAME 0xAAU
/** @brief  End of frame indicator */
#define DATAGRAM_END_FRAME 0xBBU

/**
 * @brief   Datagram metadata size
 * @details 1 byte for start frame
 *          2 bytes for ID
 *          1 byte for DLC
 *          1 byte for end frame
 */
#define DATAGRAM_METADATA_SIZE 5U

#pragma pack(push, 1)  // Ensure tight packing of the structure
typedef struct {
  uint8_t start_frame; /**< Should always be initialized to 0xAA */
  uint16_t id;         /**< CAN message ID */
  uint8_t dlc;         /**< Data Length Code: number of bytes in the message payload */
  uint8_t data[9];     /**< Holds the payload; maximum size is 8 bytes + 1 for end_frame */
} Datagram;
#pragma pack(pop)

void decode_can_message(Datagram *datagram, CanMessage *msg);

void log_decoded_message(Datagram *datagram);
