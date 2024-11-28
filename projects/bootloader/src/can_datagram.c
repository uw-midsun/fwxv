#include "can_datagram.h"

uint8_t error_buffer[DGRAM_MAX_MSG_SIZE];

// typedef struct {
//   uint32_t id;
//   uint8_t extended;
//   size_t dlc;
//   union {
//     uint64_t data;
//     uint32_t data_u32[2];
//     uint16_t data_u16[4];
//     uint8_t data_u8[8];
//   };
// } Boot_CanMessage;

// typedef struct {
//   uint8_t datagram_type_id;
//   union {
//     struct {
//       uint16_t node_ids;
//       uint32_t data_len;
//     } start;
//     struct {
//       uint16_t sequence_num;
//       uint32_t crc32;
//     } sequencing;
//     struct {
//       uint8_t *binary_data;
//     } data;
//     struct {
//       uint16_t node_ids;
//     } jump_app;
//     struct {
//       uint8_t ack_status;
//       uint16_t bootloader_error;
//     } ack;
//     struct {

//     } ping;
//   } payload;
// } BootloaderDatagram_t;

BootloaderDatagram_t unpack_datagram(Boot_CanMessage *msg, uint16_t *target_nodes) {
  BootloaderDatagram_t ret_datagram;
  do {
    switch (msg->id) {
      case CAN_ARBITRATION_START_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.start.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        ret_datagram.payload.start.data_len =
            ((uint32_t)msg->data_u16[2] << 16) | (uint32_t)msg->data_u16[1];

        break;
      case CAN_ARBITRATION_SEQUENCING_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.sequencing.sequence_num = msg->data_u16[0];
        ret_datagram.payload.sequencing.crc32 =
            ((uint32_t)msg->data_u16[2] << 16) | (uint32_t)msg->data_u16[1];
        break;
      case CAN_ARBITRATION_FLASH_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.data.binary_data = msg->data_u8;
        break;
      case CAN_ARBITRATION_JUMP_ID:
        ret_datagram.payload.jump_app.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        break;
      case CAN_ARBITRATION_PING:
        ret_datagram.payload.ping.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        // Extracts the first 4 bits
        ret_datagram.payload.ping.req = (msg->data_u16[1] >> 12) & 0xF;
        // Extracts the next 12 bits
        ret_datagram.payload.ping.data_len = msg->data_u16[1] & 0xFFF;

        break;

      default:
        break;
    }
  } while (false);

  return ret_datagram;
}

void send_ack_datagram(bool ack, BootloaderError error) {
  error_buffer[0] = ack;
  uint16_t status_code = error;
  error_buffer[1] = (uint8_t)(error << 8);
  error_buffer[2] = (uint8_t)(error);

  boot_can_transmit(CAN_ARBITRATION_ACK_ID, false, error_buffer, sizeof(error_buffer));
}
