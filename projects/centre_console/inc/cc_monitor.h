#pragma once

#include <stdbool.h>

typedef struct LocalState {
  uint8_t drive_state;
  bool cc_toggle;
  bool regen_braking;
  uint32_t target_velocity;
} LocalState;

typedef enum DriveState {
  DRIVE = 0,
  NEUTRAL,
  REVERSE,
} DriveState;

StatusCode monitor_cruise_control();
void send_message_to_mci();
void set_local_state();
StatusCode update_state();
