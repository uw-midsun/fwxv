#pragma once

#include <stdint.h>
#include "gpio.h"
#include "status.h"
#include "uart_mcu.h"

#define UART_MAX_BUFFER_LEN 512
#define BAUDRATE 9600

typedef void (*UartRxHandler)(const uint8_t *rx_arr, size_t len, void *context);

typedef struct {
  void *context;

  volatile uint8_t tx_buf[UART_MAX_BUFFER_LEN*sizeof(uint8_t)];
  volatile uint8_t rx_buf[UART_MAX_BUFFER_LEN];

  Queue *rx_line_buf;
  char delimiter;
} UartStorage;

typedef struct {
  UartPort uart;
  Queue *rx_queue;
  void *context;
} UartRxSettings;

typedef struct {
  void *context;

  GpioAddress tx;
  GpioAddress rx;
  GpioAltFn alt_fn;
} UartSettings;

// Assumes standard 8 N 1
// Initializes buffer and queue for tx and rx on provided UartPort
StatusCode uart_init(UartPort uart, UartSettings *settings, UartStorage *storage);

// Uses a queue handler from UartRxSettings which all recieved data is written to
// If the queue is full the oldest data is dropped and overwritten
StatusCode uart_rx_init(UartRxSettings *settings);

// Sets the delimiter used to break up lines between callbacks
// Note that the default delimiter is \n
StatusCode uart_set_delimiter(UartPort uart, uint8_t delimiter);

// Non-blocking TX
StatusCode uart_tx(UartPort uart, uint8_t *tx_data, size_t len);