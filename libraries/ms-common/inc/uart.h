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
// tx_buf array must be declared statically, and have size num_items*item_size
// Initializes buffer and queue for tx and rx on provided UartPort
StatusCode uart_init(UartPort uart, UartSettings *settings, uint8_t *tx_buf);

// Uses a queue handler from UartRxSettings which all recieved data is written to
// If the queue is full the oldest data is dropped and overwritten
StatusCode uart_rx_init(UartRxSettings *settings);

// Non-blocking TX
// Returns STATUS_CODE_OK on successful tx - waits ms_to_wait to aquire mutex
// otherwise if a tx is already being performed on UartPort uart,
// return STATUS_CODE_RESOURCE_EXHAUSTED
StatusCode uart_tx(UartPort uart, uint8_t *tx_data, uint8_t len, uint16_t ms_to_wait);
