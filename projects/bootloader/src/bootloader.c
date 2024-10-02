#include "bootloader.h"
#include "boot_crc32.h"

// Store CAN traffic in 1024 byte buffer to write to flash
static uint8_t flash_buffer[BOOTLOADER_PAGE_BYTES];

static BootloaderDatagram_t datagram;
static BootloaderStateData prv_bootloader = { .state = BOOTLOADER_UNINITIALIZED,
                                              .error = BOOTLOADER_ERROR_NONE,
                                              .first_byte_received = false };

BootloaderError bootloader_init() {
  prv_bootloader.bytes_written = 0;
  prv_bootloader.binary_size = 0;
  prv_bootloader.application_start = APP_START_ADDRESS;
  prv_bootloader.current_address = APP_START_ADDRESS;
  prv_bootloader.expected_sequence_number = 0;
  prv_bootloader.packet_crc32 = 0;
  prv_bootloader.state = BOOTLOADER_IDLE;
  prv_bootloader.target_nodes = 0;
  prv_bootloader.buffer_index = 0;

  boot_crc_init();

  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_switch_states(const BootloaderStates new_state) {
  BootloaderError return_err = BOOTLOADER_ERROR_NONE;
  BootloaderStates current_state = prv_bootloader.state;
  if (current_state == new_state) {
    return return_err;
  }

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
      if (new_state == BOOTLOADER_START || new_state == BOOTLOADER_JUMP_APP ||
          new_state == BOOTLOADER_DATA_READY || new_state == BOOTLOADER_FAULT) {
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

static BootloaderError bootloader_handle_arbitration_id(Boot_CanMessage *msg) {
  switch (msg->id) {
    case CAN_ARBITRATION_START_ID:
      return bootloader_switch_states(BOOTLOADER_START);
    case CAN_ARBITRATION_SEQUENCING_ID:
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
  prv_bootloader.buffer_index = 0;
  prv_bootloader.error = 0;
  prv_bootloader.first_byte_received = false;
  prv_bootloader.expected_sequence_number = 0;

  if (prv_bootloader.binary_size % BOOTLOADER_WRITE_BYTES != 0) {
    send_ack_datagram(false, BOOTLOADER_DATA_NOT_ALIGNED);
    return BOOTLOADER_DATA_NOT_ALIGNED;
  }

  send_ack_datagram(true, BOOTLOADER_ERROR_NONE);
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_jump_app() {
  send_ack_datagram(true, BOOTLOADER_ERROR_NONE);
  __asm volatile(
      "LDR     R0, =prv_bootloader  \n"
      "LDR     R1, [R0]             \n"
      "LDR     R2, [R1, #4]         \n"
      "BX      R2                   \n");

  return BOOTLOADER_ERROR_NONE;  // Should trigger error
}

static BootloaderError bootloader_data_ready() {
  if (prv_bootloader.expected_sequence_number == datagram.payload.sequencing.sequence_num) {
    prv_bootloader.packet_crc32 = datagram.payload.sequencing.crc32;
    prv_bootloader.buffer_index = 0;
    if (datagram.payload.sequencing.sequence_num == 0) {
      prv_bootloader.first_byte_received = true;
      prv_bootloader.bytes_written = 0;
      prv_bootloader.current_address = APP_START_ADDRESS;
    }
    prv_bootloader.expected_sequence_number++;
  } else {
    send_ack_datagram(false, BOOTLOADER_SEQUENCE_ERROR);
    return BOOTLOADER_SEQUENCE_ERROR;
  }
  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_data_receive() {
  BootloaderError error = BOOTLOADER_ERROR_NONE;
  if (!prv_bootloader.first_byte_received) {
    send_ack_datagram(false, BOOTLOADER_INTERNAL_ERR);
    return BOOTLOADER_INTERNAL_ERR;
  }
  if (prv_bootloader.buffer_index >= BOOTLOADER_PAGE_BYTES) {
    send_ack_datagram(false, BOOTLOADER_BUFFER_OVERFLOW);
    return BOOTLOADER_BUFFER_OVERFLOW;
  }

  size_t bytes_to_copy = 8;
  if (BOOTLOADER_PAGE_BYTES - prv_bootloader.buffer_index < 8) {
    bytes_to_copy = BOOTLOADER_PAGE_BYTES - prv_bootloader.buffer_index;
  }
  if (prv_bootloader.binary_size - (prv_bootloader.buffer_index + prv_bootloader.bytes_written) < 8) {
    bytes_to_copy = prv_bootloader.binary_size - (prv_bootloader.buffer_index + prv_bootloader.bytes_written);
  }

  memcpy(flash_buffer + prv_bootloader.buffer_index, datagram.payload.data.binary_data, bytes_to_copy);
  prv_bootloader.buffer_index += bytes_to_copy;

  if (prv_bootloader.buffer_index == BOOTLOADER_PAGE_BYTES ||
      (prv_bootloader.bytes_written + prv_bootloader.buffer_index)  >= prv_bootloader.binary_size) {

    uint32_t calculated_crc32 = boot_crc_calculate((const uint32_t *)flash_buffer, BYTES_TO_WORD(prv_bootloader.buffer_index));
    if (calculated_crc32 != prv_bootloader.packet_crc32) {
      send_ack_datagram(false, BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE);
      return BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE;
    }
    
    error |= boot_flash_erase(BOOTLOADER_ADDR_TO_PAGE(prv_bootloader.current_address));
    error |= boot_flash_write(prv_bootloader.current_address, flash_buffer, BOOTLOADER_PAGE_BYTES);
    error |= boot_flash_read(prv_bootloader.current_address, flash_buffer, BOOTLOADER_PAGE_BYTES);

    if (error != BOOTLOADER_ERROR_NONE) {
      send_ack_datagram(false, error);
      return error;
    }

    calculated_crc32 = boot_crc_calculate((uint32_t *)flash_buffer, BYTES_TO_WORD(prv_bootloader.buffer_index));
    if (calculated_crc32 != prv_bootloader.packet_crc32) {
      send_ack_datagram(false, BOOTLOADER_CRC_MISMATCH_AFTER_WRITE);
      return BOOTLOADER_CRC_MISMATCH_AFTER_WRITE;
    }

    prv_bootloader.bytes_written += prv_bootloader.buffer_index;
    prv_bootloader.current_address += prv_bootloader.buffer_index;
    prv_bootloader.buffer_index = 0;
    memset(flash_buffer, 0, sizeof(flash_buffer));
    send_ack_datagram(true, BOOTLOADER_ERROR_NONE);
    if (prv_bootloader.bytes_written >= prv_bootloader.binary_size) {
      return bootloader_jump_app();
    }
  }
  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_fault() {
  /* Implement code to reset the board. */
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

BootloaderError bootloader_run(Boot_CanMessage *msg) {
  BootloaderError ret = BOOTLOADER_ERROR_NONE;

  if (prv_bootloader.state == BOOTLOADER_UNINITIALIZED) {
    return BOOTLOADER_ERROR_UNINITIALIZED;
  }

  datagram = unpack_datagram(msg, &prv_bootloader.target_nodes);

  // if (!(prv_bootloader.target_nodes & (1 << _node_id))) {
  //   // Not the target node
  //   return BOOTLOADER_ERROR_NONE;
  // }

  ret = bootloader_handle_arbitration_id(msg);
  ret = bootloader_run_state();

  if (ret != BOOTLOADER_ERROR_NONE) {
    bootloader_switch_states(BOOTLOADER_FAULT);
  }

  return ret;
}