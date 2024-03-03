#include "uart.h"

#include <string.h>

// NOT COMPLETED

typedef struct {
  UartSettings settings;
  uint8_t tx_buf[UART_MAX_BUFFER_LEN];
  Queue tx_queue;
  uint8_t rx_buf[UART_MAX_BUFFER_LEN];
  Queue rx_queue;

  bool initialized;
} UartPortData;

static UartPortData s_port[NUM_UART_PORTS] = {
  [UART_PORT_1] = {}, [UART_PORT_2] = {}, [UART_PORT_3] = {}, [UART_PORT_4] = {}
};

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  if (uart >= NUM_UART_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid UART port.");
  }
  if (s_port[uart].initialized)
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Already initialized.");

  s_port[uart].settings = *settings;

  s_port[uart].tx_queue.item_size = sizeof(uint8_t);
  s_port[uart].tx_queue.num_items = UART_MAX_BUFFER_LEN;
  s_port[uart].tx_queue.storage_buf = s_port[uart].tx_buf;
  status_ok_or_return(queue_init(&s_port[uart].tx_queue));

  s_port[uart].rx_queue.item_size = sizeof(uint8_t);
  s_port[uart].rx_queue.num_items = UART_MAX_BUFFER_LEN;
  s_port[uart].rx_queue.storage_buf = s_port[uart].rx_buf;
  status_ok_or_return(queue_init(&s_port[uart].rx_queue));

  gpio_init_pin(&settings->tx, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->rx, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

  s_port[uart].initialized = true;
  return STATUS_CODE_OK;
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t *len) {
  for (size_t i = 0; i < *len; i++) {
    if (queue_receive(&s_port[uart].rx_queue, data, 0)) {
      queue_reset(&s_port[uart].rx_queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode uart_set_delimiter(UartPort uart, uint8_t delimiter) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t *len) {
  for (size_t i = 0; i < *len; i++) {
    if (queue_send(&s_port[uart].tx_queue, data, 0)) {
      queue_reset(&s_port[uart].tx_queue);
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }
  return STATUS_CODE_OK;
}
