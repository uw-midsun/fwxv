
#include <stdbool.h>
#include <stdint.h>

#include "motor_can.h"
#include "motor_controller_getters.h"

#define ACCERLATION_FORCE 100
#define CRUISE_THROTTLE_THRESHOLD 0

typedef enum DriveState {
  // drive states defined by center console
  DRIVE,
  NEUTRAL,
  REVERSE,
  // extra drive state types used only by mci
  CRUISE,
  BREAK,
} DriveState;

void process_data() {
  // verify that can messages from center console, peddal are not stale
  s_send_message = true;
  if (!s_send_message) {
    return;
  }

  uint32_t throttle_percent = get_pedal_throttle_output();
  uint32_t break_percent = get_pedal_brake_output();
  uint32_t target_vel = get_drive_output_target_velocity();

  // TODO: fix autogen to allow mis-matched signal length
  // currently just use masks to get the values within drive state
  DriveState drive_state = (get_drive_output_drive_state() & 0xFF000000) >> 24;
  bool regen = (get_drive_output_drive_state() & 0x00FF0000) >> 16;
  bool cruise = (get_drive_output_drive_state() & 0x0000FF00) >> 8;

  if (cruise && throttle_percent > CRUISE_THROTTLE_THRESHOLD) {
    drive_state = DRIVE;
  }
  if (break_percent > 0 || throttle_percent == 0) {
    drive_state = regen ? BREAK : NEUTRAL;
  }

  switch (drive_state) {
    case DRIVE:
      s_target_current = throttle_percent;
      s_target_velocity = 2000;
      break;
    case REVERSE:
      s_target_current = throttle_percent;
      s_target_velocity = -2000;
      break;
    case CRUISE:
      s_target_current = ACCERLATION_FORCE;
      s_target_velocity = target_vel;
      break;
    case BREAK:
      s_target_current = ACCERLATION_FORCE;
      s_target_velocity = 0;
      break;
    case NEUTRAL:
      s_target_current = 0;
      s_target_velocity = 0;
      break;
    default:
      // invalid drive state
      return;
  }
}
