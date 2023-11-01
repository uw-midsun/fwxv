#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "status.h"

#define STEERING_CC_TOGGLE_MASK 0x1
#define STEERING_CC_INCREASE_SPEED_MASK 0x2
#define STEERING_CC_DECREASE_SPEED_MASK 0x4

typedef struct LocalState {
  uint8_t drive_state;
  bool cc_enabled;
  bool regen_braking;
  uint32_t target_velocity;
} LocalState;

void monitor_cruise_control();
