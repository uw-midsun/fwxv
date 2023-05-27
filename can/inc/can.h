#pragma once
// CAN application interface
// Requires GPIO, soft timers, event queue, and interrupts to be initialized.
//
// Application code should only use functions in this header.
// Note that the CAN FSM must be clocked. Call `can_process_event(&e)` in your
// event loop.
//
#include <stdbool.h>
// for memset
#include <string.h>
#include <stdint.h>
#include "status.h"
#include "can_msg.h"
// #include "can_ack.h"
#include "can_hw.h"

typedef uint8_t EventId;

typedef struct CanStorage {
  volatile CanQueue rx_queue;
  uint16_t device_id;
} CanStorage;

// Initializes the specified CAN configuration.
StatusCode can_init(CanStorage *storage, const CanSettings *settings);

// Adds a hardware filter in for the specified message ID.
StatusCode can_add_filter_in(CanMessageId msg_id);

// Adds a hardware filter out for the specified message ID.
StatusCode can_add_filter_out(CanMessageId msg_id);

// Attempts to transmit the CAN message as soon as possible.
StatusCode can_transmit(const CanMessage *msg);

// Attempts to receive the CAN message as soon as possible.
StatusCode can_receive(const CanMessage *msg);

// Run the can rx cycle
StatusCode run_can_rx_cycle();

// Run the can tx cycle
StatusCode run_can_tx_cycle();

// Clear RX struct
StatusCode clear_rx_struct();

// Clear TX struct
StatusCode clear_tx_struct();
