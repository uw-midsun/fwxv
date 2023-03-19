#include "i2c.h"

#include <stdbool.h>

#include "log.h"
#include "queues.h"
#include "semaphore.h"
#include "stdio.h"

typedef enum I2CMode {
  I2C_MODE_TRANSMIT = 0,
  I2C_MODE_RECEIVE,
  NUM_I2C_MODES,
} I2CMode;

// Used to protect I2C resources
// All data tx'd and rx'd through queue
// Transactions on each I2C port protected by Mutex
typedef struct {
  uint8_t buf[I2C_MAX_NUM_DATA];
  Queue queue;
  Mutex mutex;
} I2CBuffer;

typedef struct {
  I2CSettings settings;           // I2C Config
  I2CBuffer i2c_tx_buf;           // RTOS data structures
  I2CBuffer i2c_rx_buf;           // RTOS data structures
  bool curr_mode;                 // Mode used in current transxn
  I2CAddress current_addr;        // Addr used in current transxn
  volatile uint8_t num_rx_bytes;  // Number of bytes left to receive in rx mode
} I2CPortData;

static I2CPortData s_port[NUM_I2C_PORTS] = {
  [I2C_PORT_1] = {},
  [I2C_PORT_2] = {},
};

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  } else if (settings->speed >= NUM_I2C_SPEEDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C speed.");
  }

  s_port[i2c].settings = *settings;

  s_port[i2c].i2c_rx_buf.queue.num_items = I2C_MAX_NUM_DATA;
  s_port[i2c].i2c_rx_buf.queue.item_size = sizeof(uint8_t);
  s_port[i2c].i2c_rx_buf.queue.storage_buf = s_port[i2c].i2c_rx_buf.buf;
  status_ok_or_return(queue_init(&s_port[i2c].i2c_rx_buf.queue));

  s_port[i2c].i2c_tx_buf.queue.num_items = I2C_MAX_NUM_DATA;
  s_port[i2c].i2c_tx_buf.queue.item_size = sizeof(uint8_t);
  s_port[i2c].i2c_tx_buf.queue.storage_buf = s_port[i2c].i2c_tx_buf.buf;
  status_ok_or_return(queue_init(&s_port[i2c].i2c_tx_buf.queue));

  return STATUS_CODE_OK;
}

#ifdef MS_PLATFORM_X86
StatusCode i2c_set_data(I2CPort i2c, uint8_t *tx_data, size_t tx_len) {
  for (size_t tx = 0; tx < tx_len; tx++) {
    if (queue_send(&s_port[i2c].i2c_rx_buf.queue, &tx_data[tx], 0)) {
      queue_reset(&s_port[i2c].i2c_rx_buf.queue);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_get_data(I2CPort i2c, uint8_t *rx_data, size_t rx_len) {
  for (size_t rx = 0; rx < rx_len; rx++) {
    if (queue_receive(&s_port[i2c].i2c_tx_buf.queue, &rx_data[rx], 0)) {
      queue_reset(&s_port[i2c].i2c_tx_buf.queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}
#endif

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }

  // Set number of bytes to read
  s_port[i2c].num_rx_bytes = rx_len;
  // Store address for this transaction
  s_port[i2c].current_addr = addr;
  s_port[i2c].curr_mode = I2C_MODE_RECEIVE;

  // Receive data from queue
  // If less than requested is received, an error in the transaction has occurred
  for (size_t rx = 0; rx < rx_len; rx++) {
    if (queue_receive(&s_port[i2c].i2c_rx_buf.queue, &rx_data[rx], 0)) {
      queue_reset(&s_port[i2c].i2c_rx_buf.queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }

  // Copy data into queue
  for (size_t tx = 0; tx < tx_len; tx++) {
    if (queue_send(&s_port[i2c].i2c_tx_buf.queue, &tx_data[tx], 0)) {
      queue_reset(&s_port[i2c].i2c_tx_buf.queue);
      return STATUS_CODE_RESOURCE_EXHAUSTED;
    }
  }

  // Store address for this transaction
  s_port[i2c].current_addr = addr;
  s_port[i2c].curr_mode = I2C_MODE_TRANSMIT;

  return STATUS_CODE_OK;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                        size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  uint8_t reg_to_read = reg;
  status_ok_or_return(i2c_write(i2c, addr, &reg_to_read, sizeof(reg)));
  status_ok_or_return(i2c_read(i2c, addr, rx_data, rx_len));
  return STATUS_CODE_OK;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }

  uint8_t reg_to_write = reg;
  status_ok_or_return(i2c_write(i2c, addr, &reg_to_write, sizeof(reg)));
  status_ok_or_return(i2c_write(i2c, addr, tx_data, tx_len));
  return STATUS_CODE_OK;
}
