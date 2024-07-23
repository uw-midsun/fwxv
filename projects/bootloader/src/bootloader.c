#include "bootloader.h"

static uint8_t flash_buffer[8];

static can_datagram_t datagram;
static BootloaderStateData prv_bootloader = { .state = BOOTLOADER_UNINITIALIZED,
                                              .error = BOOTLOADER_ERROR_NONE,
                                              .first_byte_received = false };

BootloaderError bootloader_init() {
  size_t page = 0;
  for (page = 1 + ((APP_START_ADDRESS - 0x08000000) / BOOTLOADER_PAGE_BYTES); page < NUM_FLASH_PAGES - 1; page++) {
    if (boot_flash_erase(page)) {
      return BOOTLOADER_FLASH_ERR;
    }
  }
  prv_bootloader.bytes_written = 0;
  prv_bootloader.binary_size = 0;
  prv_bootloader.current_address = APP_START_ADDRESS;
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
      if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_DATA_READY ||
          new_state == BOOTLOADER_FAULT) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_DATA_READY:
      // Should be able to go to all states
      prv_bootloader.state = new_state;
      break;

    case BOOTLOADER_DATA_RECEIVE:
      if (new_state == BOOTLOADER_START || new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_FAULT) {
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
    case CAN_ARBITRATION_START_FLASH_ID:
      return bootloader_switch_states(BOOTLOADER_DATA_READY);
    case CAN_ARBITRATION_FLASH_ID:
      return bootloader_switch_states(BOOTLOADER_DATA_RECEIVE);
    case CAN_ARBITRATION_JUMP_ID:
      return bootloader_switch_states(BOOTLOADER_JUMP_APP);
    default:
      return BOOTLOADER_INVALID_ARGS;
  }
}

static BootloaderError bootloader_start() {
  prv_bootloader.binary_size = datagram.payload.start.data_len;
  prv_bootloader.bytes_written = 0;
  prv_bootloader.error = 0;
  prv_bootloader.first_byte_received = false;

  if (prv_bootloader.binary_size % BOOTLOADER_WRITE_BYTES != 0) {
    return BOOTLOADER_DATA_NOT_ALIGNED;
  }
  

  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_jump_app() {
  // Adding 32 bits from the applications main stack pointer to pull the function from
  // the application reset handler (Reset vector)
  void (*app_reset_handler)(void) = (void *)(volatile uint32_t *)(APP_START_ADDRESS + 0x4);
  
  // Updating main stack pointer to be the value stored at APP_START_ADDRESS
  // __set_MSP(*(volatile uint32_t *)APP_START_ADDRESS);

  // app_reset_handler();

  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_data_ready() {
  uint8_t buffer_len = 0;

  if (prv_bootloader.binary_size - prv_bootloader.bytes_written == BOOTLOADER_WRITE_BYTES) {
    buffer_len = BOOTLOADER_WRITE_BYTES;
  } else {
    buffer_len = sizeof(datagram.payload.data);
  }

  // boot_flash_write(prv_bootloader.current_address, datagram.payload.data.binary_data, buffer_len);
  prv_bootloader.bytes_written += buffer_len;
  prv_bootloader.current_address += prv_bootloader.bytes_written;
  prv_bootloader.first_byte_received = true;
  return BOOTLOADER_ERROR_NONE;

}

static BootloaderError bootloader_data_receive() {
  if (!prv_bootloader.first_byte_received) {
    return BOOTLOADER_INTERNAL_ERR;
  }

  uint8_t buffer_len = 0;

  if (prv_bootloader.binary_size - prv_bootloader.bytes_written == BOOTLOADER_WRITE_BYTES) {
    buffer_len = BOOTLOADER_WRITE_BYTES;
  } else {
    buffer_len = sizeof(datagram.payload.data);
  }

  // boot_flash_write(prv_bootloader.current_address, datagram.payload.data.binary_data, buffer_len);
  prv_bootloader.bytes_written += buffer_len;
  prv_bootloader.current_address += prv_bootloader.bytes_written;

  return BOOTLOADER_ERROR_NONE;
};

static BootloaderError bootloader_fault() {
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
    case BOOTLOADER_START:
      return bootloader_start();
      break;
    case BOOTLOADER_DATA_READY:
      return bootloader_data_ready();
    case BOOTLOADER_DATA_RECEIVE:
      return bootloader_data_receive();
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
  *datagram = unpack_datagram(msg, prv_bootloader.first_byte_received);

  if (!prv_bootloader.first_byte_received && msg->id.raw == CAN_ARBITRATION_START_FLASH_ID) {
    prv_bootloader.first_byte_received = true;
  }

  if (datagram->datagram_type_id == CAN_ARBITRATION_START_ID) {
    prv_bootloader.binary_size = datagram->payload.start.data_len;
  }

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_run(CanMessage *msg) {
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