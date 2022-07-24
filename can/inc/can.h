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
typedef uint8_t CanAckRequest;
// typedef CanAckRequest[8] CanAckRequests;

typedef struct CanStorage {
  // volatile CanFifo tx_fifo;
  volatile CanQueue rx_queue;
  CanAckRequest ack_requests;
  uint16_t device_id;
} CanStorage;

// Initializes the specified CAN configuration.
StatusCode can_init(CanStorage *storage, const CanSettings *settings);

// Adds a hardware filter in for the specified message ID.
StatusCode can_add_filter_in(CanMessageId msg_id);

// Adds a hardware filter out for the specified message ID.
StatusCode can_add_filter_out(CanMessageId msg_id);

// Attempts to transmit the CAN message as soon as possible.
StatusCode can_transmit(const CanMessage *msg, const CanAckRequest *ack_request);

// Attempts to receive the CAN message as soon as possible.
StatusCode can_receive(const CanMessage *msg, const CanAckRequest *ack_request);
