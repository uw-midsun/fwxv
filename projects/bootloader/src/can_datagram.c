#include "can_datagram.h"

uint8_t error_buffer[DGRAM_MAX_MSG_SIZE];

can_datagram_t unpack_datagram(Boot_CanMessage *msg, uint16_t *target_nodes) {
  can_datagram_t ret_datagram;
  do {
    switch (msg->id) {
      case CAN_ARBITRATION_START_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.start.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        ret_datagram.payload.start.data_len =
            ((uint32_t)msg->data_u16[2] << 16) | (uint32_t)msg->data_u16[1];

        break;
      case CAN_ARBITRATION_START_FLASH_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.data.binary_data = msg->data_u8;
        break;
      case CAN_ARBITRATION_FLASH_ID:
        ret_datagram.datagram_type_id = msg->id;
        ret_datagram.payload.data.binary_data = msg->data_u8;
        break;
      case CAN_ARBITRATION_JUMP_ID:
        ret_datagram.payload.jump_app.node_ids = msg->data_u16[0];
        *target_nodes = msg->data_u16[0];
        break;
      default:
        break;
    }
  } while (false);

  return ret_datagram;
}

BootloaderError can_datagram_transmit_error(BootloaderError error) {
  // [ BootloaderError, TBD ]
  error_buffer[0] = error;
  boot_can_transmit(CAN_ARBITRATION_FAULT_ID, false, error_buffer, sizeof(error_buffer));

  return BOOTLOADER_ERROR_NONE;
}
