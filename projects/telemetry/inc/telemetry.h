#pragma once

#include "datagram.h"
#include "stack.h"
#include "status.h"
#include "uart.h"

typedef struct {
  UartPort uart_port;         /**< UART Port for the telemetry interface (XBee, ELRS, etc.) */
  UartSettings uart_settings; /**< UART Settings for the telemetry interface */
  uint32_t message_transmit_frequency_hz; /**< Transmission frequency of telemetry data packets */
} TelemetryConfig;

typedef struct {
  Datagram datagram_buffer[DATAGRAM_BUFFER_SIZE]; /**< Buffer to store the datagram */
  Stack datagram_stack;                           /**< Queue handle for the datagram buffer */
  TelemetryConfig *config; /**< Pointer to the telemetry configuration data */
} TelemetryStorage;

StatusCode telemetry_init(TelemetryStorage *storage, TelemetryConfig *config);
