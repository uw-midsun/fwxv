#pragma once
// CAN Debug library
//
// Allows callback functions to be registered with specific CAN Message IDs.
//
// See fwxv/projects/can_debug for a reference project.

#include "can.h"

// The maximum number of callbacks that can be registered
#define MAX_NUM_CALLBACKS 5

typedef void (*CanDebugCallback)(uint64_t);

// Resets the callback function counter and memsets the CAN message to 0
StatusCode can_debug_init();

// Registers a callback with a specific CAN Message ID
StatusCode can_debug_register(CanMessageId id, CanDebugCallback callback);

// Checks for registered CAN Message IDs and calls the associated callback
void check_can_messages();