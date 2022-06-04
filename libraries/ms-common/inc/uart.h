#pragma once

#include <stdint.h>
#include "gpio.h"
#include "queues.h"
#include "status.h"
#include "uart_mcu.h"

// Module is not thread safe - do not access a single port from
// two locations at the same time

#define UART_MAX_BUFFER_LEN 512

typedef struct {
  GpioAddress tx;  // tx pin
  GpioAddress rx;  // rx pin
  GpioAltFn alt_fn;
  uint32_t baudrate;
} UartSettings;

// Assumes standard 8 N 1
// Port 1 is reserved for retarget.c
StatusCode uart_init(UartPort uart, UartSettings *settings);

// RX data is read to *data with length returned as *len
// *data read into and overwritten from ISR queue buffer when function is called
StatusCode uart_rx(UartPort uart, uint8_t *data, size_t *len);

// Non-blocking TX
// Returns STATUS_CODE_RESOURCE_EXHAUSTED if mutex is in use
// *len returns number of items read
StatusCode uart_tx(UartPort uart, uint8_t *data, size_t *len);
