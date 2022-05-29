#pragma once

#include <stdint.h>
#include "gpio.h"
#include "queues.h"
#include "status.h"
#include "uart_mcu.h"

#define UART_MAX_BUFFER_LEN 512
#define BAUDRATE 9600

typedef struct {
  UartPort uart;
  Queue *rx_queue;
} UartRxSettings;

typedef struct {
  void *context;

  GpioAddress tx;  // tx pin
  GpioAddress rx;  // rx pin
  GpioAltFn alt_fn;
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
