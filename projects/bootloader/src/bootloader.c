#include "bootloader.h"

static BootloaderStateData prv_bootloader = { .state = BOOTLOADER_UNINITIALIZED,
                                              .error = BOOTLOADER_ERROR_NONE,
                                              .first_byte_received = false };

BootloaderError bootloader_init() {
    for (size_t page = 1; page < NUM_FLASH_PAGES - 1; page++) {
        if (boot_flash_erase(page)) {
            return BOOTLOADER_FLASH_ERR;
        }
    }
    prv_bootloader.bytes_received = 0;
    prv_bootloader.binary_size = 0;
    prv_bootloader.state = BOOTLOADER_IDLE;

    return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_switch_states(const BootloaderStates new_state) {
  BootloaderError return_err = BOOTLOADER_ERROR_NONE;
  BootloaderStates current_state = prv_bootloader.state;

  switch (current_state) {
    case BOOTLOADER_IDLE:
      if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_START ||
          new_state == BOOTLOADER_FAULT) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_START:
        if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_DATA_RECEIVE || new_state == BOOTLOADER_FAULT) {
            prv_bootloader.state = new_state;
        } else {
            return_err = BOOTLOADER_INVALID_ARGS;
            prv_bootloader.state = BOOTLOADER_FAULT;
        }
        break;

    case BOOTLOADER_DATA_RECEIVE:
      if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_FAULT) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_JUMP_APP:
      if (new_state == BOOTLOADER_FAULT) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_UNINITIALIZED:
      if (new_state == BOOTLOADER_IDLE) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    default:
      return_err = BOOTLOADER_INVALID_ARGS;
      prv_bootloader.state = BOOTLOADER_FAULT;
      break;
  }

  return return_err;
}

static BootloaderError bootloader_handle_arbitration_id(CanMessage *msg) {
    switch (msg->id.raw) {
        case CAN_ARBITRATION_START_ID:
            return bootloader_switch_states(BOOTLOADER_START);
        case CAN_ARBITRATION_FLASH_ID:
            return bootloader_switch_states(BOOTLOADER_DATA_RECEIVE);
        case CAN_ARBITRATION_JUMP_ID:
            return bootloader_switch_states(BOOTLOADER_JUMP_APP);
        default:
            return BOOTLOADER_INVALID_ARGS;
    }
}

BootloaderError bootloader_jump_app() {
  BootloaderStates curr_state = prv_bootloader.state;

  if (curr_state != BOOTLOADER_IDLE) {
  }
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_data() {
    
    return BOOTLOADER_ERROR_NONE;
};

BootloaderError bootloader_fault() {
    return BOOTLOADER_INTERNAL_ERR;
};

static BootloaderError bootloader_run_state() {
    switch (prv_bootloader.state) {
        case BOOTLOADER_UNINITIALIZED:
            return BOOTLOADER_INVALID_ARGS;
            break;
        case BOOTLOADER_IDLE:
            return BOOTLOADER_ERROR_NONE;
            break;
        case BOOTLOADER_DATA_RECEIVE:
            return bootloader_data();
        case BOOTLOADER_JUMP_APP:
            return bootloader_jump_app();
        case BOOTLOADER_FAULT:
            return bootloader_fault();
        default:
            return BOOTLOADER_INTERNAL_ERR;
    }
}

BootloaderError bootloader_get_err() {
  return prv_bootloader.error;
}
BootloaderStates bootloader_get_state(void) {
  return prv_bootloader.state;
}

static BootloaderError bootloader_unpack_message(CanMessage *msg, can_datagram_t *datagram) {
  if (!prv_bootloader.first_byte_received && msg->id.raw == CAN_ARBITRATION_FLASH_ID) {
    if (msg->data_u8[0] != CAN_DATAGRAM_SOF) {
      // Generate Error message
      return BOOTLOADER_INVALID_ID;
    }
  }
  *datagram = unpack_datagram(msg, prv_bootloader.first_byte_received);

  if (datagram->datagram_type_id == CAN_ARBITRATION_START_ID) {
    prv_bootloader.binary_size = datagram->payload.start.data_len;
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_run(CanMessage *msg) {
  can_datagram_t datagram;
  BootloaderError ret = BOOTLOADER_ERROR_NONE;
  
  if (bootloader_unpack_message(msg, &datagram)) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  if (datagram.datagram_type_id == CAN_ARBITRATION_START_ID) {
    if (!(datagram.payload.start.node_ids & NODE_ID)) {
      // Return because this device is not the target
      return BOOTLOADER_ERROR_NONE;
    }
  }
  
  ret = bootloader_handle_arbitration_id(msg);
  ret = bootloader_run_state();

  return ret;
}