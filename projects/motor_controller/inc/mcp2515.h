#pragma once
// CAN application interface
// Requires GPIO, soft timers, event queue, and interrupts to be initialized.
//
// Application code should only use functions in this header.
// Note that the CAN FSM must be clocked. Call `can_process_event(&e)` in your
// event loop.
//
#include "can.h"
#include "gpio.h"
#include "spi.h"

typedef struct Mcp2515Errors {
  uint8_t eflg;
  uint8_t tec;
  uint8_t rec;
} Mcp2515Errors;

typedef struct Mcp2515Settings {
  SpiPort spi_port;
  SpiSettings spi_settings;

  GpioAddress interrupt_pin;

  // Mcp does not support 1000kbps bitrate
  CanSettings can_settings;
} Mcp2515Settings;

typedef struct Mcp2515Storage {
  SpiPort spi_port;
  volatile CanQueue rx_queue;
  Mcp2515Errors errors;
} Mcp2515Storage;

// Initializes the specified CAN configuration.
StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings);

// Adds a hardware filter in for the specified message ID.
// StatusCode mcp2515_add_filter_in(CanMessageId msg_id);

// Adds a hardware filter out for the specified message ID.
// StatusCode mcp2515_add_filter_out(CanMessageId msg_id);

StatusCode mcp2515_set_filter(CanMessageId *filters, bool loopback);

// Attempts to transmit the CAN message as soon as possible.
StatusCode mcp2515_transmit(const CanMessage *msg);

// Attempts to receive the CAN message as soon as possible.
StatusCode mcp2515_receive(const CanMessage *msg);

// Run the can rx cycle
StatusCode mcp2515_can_rx_cycle();

// Run the can tx cycle
StatusCode mcp2515_can_tx_cycle();
