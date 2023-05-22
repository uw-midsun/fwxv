#include "uart.h"

StatusCode uart_init(UartPort uart, UartSettings *settings) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_rx(UartPort uart, uint8_t *data, size_t *len) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_set_delimiter(UartPort uart, uint8_t delimiter) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_tx(UartPort uart, uint8_t *data, size_t *len) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
