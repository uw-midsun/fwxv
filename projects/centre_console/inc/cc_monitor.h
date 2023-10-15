#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "status.h"

typedef struct LocalState {
  uint8_t drive_state;
  bool cc_toggle;
  bool regen_braking;
  uint32_t target_velocity;
} LocalState;

typedef enum {
  STEERING_REGEN_BRAKE_EVENT = 0,
  STEERING_CC_TOGGLE_EVENT,
  STEERING_CC_INCREASE_SPEED_EVENT,
  STEERING_CC_DECREASE_SPEED_EVENT,
  NUM_STEERING_EVENTS,
} SteeringDigitalEvent;

void monitor_cruise_control();
void send_message_to_mci();
void set_local_state();
void update_state();
