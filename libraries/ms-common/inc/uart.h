#pragma once

#include "gpio.h"
#include "queues.h"
#include "status.h"
#include "uart_mcu.h"

// STM32 UART interface module
// Gpios and interrupts must be initialized to use
// Module is not thread safe - do not access a single port from
// two locations at the same time

#define UART_MAX_BUFFER_LEN 512

typedef struct {
  GpioAddress tx;  // tx pin
  GpioAddress rx;  // rx pin
  uint32_t baudrate;
} UartSettings;

// Assumes standard 8 N 1
// Port 1 is reserved for retarget.c
StatusCode uart_init(UartPort uart, UartSettings *settings);

// Reads |*len| bytes of data into |*data| from UART rx queue at port |port|, up to
// UART_MAX_BUFFER_LEN. If less than |*len| available, it is updated to number of bytes read. If rx
// data buffer is full, oldest data is removed from front of queue. This method should not be called
// from more than one task for the same port
StatusCode uart_rx(UartPort uart, uint8_t *data, size_t *len);

// Sends |*len| bytes of data from |*data| UART tx queue at port |port|,
// up to UART_MAX_BUFFER_LEN or if queue is full.
// If less than |*len| sent, it is updated to total bytes sent.
// This method should not be called from more than one task for the same port
StatusCode uart_tx(UartPort uart, uint8_t *data, size_t *len);
