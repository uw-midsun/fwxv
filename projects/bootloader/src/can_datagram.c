#include "can_datagram.h"

can_datagram_t unpack_datagram(CanMessage *msg, bool first_byte_received) {
  can_datagram_t ret_datagram;
  do {
    switch (msg->id.raw) {
      case CAN_ARBITRATION_START_ID:
        ret_datagram.datagram_type_id = msg->id.raw;
        ret_datagram.payload.start.node_ids = msg->data_u16[0];
        ret_datagram.payload.start.data_len = msg->data_u16[1];
        break;
      case CAN_ARBITRATION_START_FLASH_ID:
        ret_datagram.datagram_type_id = msg->id.raw;
        ret_datagram.payload.data.binary_data = msg->data_u8;
        break;
      case CAN_ARBITRATION_FLASH_ID:
        if (first_byte_received) {
          ret_datagram.datagram_type_id = msg->id.raw;
          ret_datagram.payload.data.binary_data = msg->data_u8;
        }
        break;
      case CAN_ARBITRATION_JUMP_ID:
        break;
      default:
        break;
    }
  } while (false);

  return ret_datagram;
}
