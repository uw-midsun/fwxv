
#include <stdbool.h>
#include <stdint.h>

#include "motor_can.h"
#include "motor_controller_getters.h"

#define ACCERLATION_FORCE 1
#define CRUISE_THROTTLE_THRESHOLD 0
#define TORQUE_CONTROL_VEL 20000

typedef enum DriveState {
  // drive states defined by center console
  DRIVE,
  NEUTRAL,
  REVERSE,
  // extra drive state types used only by mci
  CRUISE,
  BREAK,
} DriveState;

static uint32_t vel_to_rpm(uint32_t f) {
  // convert to float
  union {
    float f;
    uint32_t u;
  } fu = { .f = f };

  // TODO: set actual ratio, m/s to motor rpm
  float ratio = 1;

  return fu.u * ratio;
}

static float get_float(uint32_t f) {
  union {
    float f;
    uint32_t u;
  } fu = { .f = f };
  return fu.u;
}

void process_data() {
  // verify that can messages from center console, pedal are not stale
  s_send_message = true;
  if (!s_send_message) {
    return;
  }

  float throttle_percent = get_float(get_pedal_throttle_output());
  float break_percent = get_float(get_pedal_brake_output());
  float target_vel = vel_to_rpm(get_drive_output_target_velocity());

  DriveState drive_state = get_drive_output_drive_state();
  bool regen = get_drive_output_regen_braking();
  bool cruise = get_drive_output_cruise_control();

  if (cruise && throttle_percent > CRUISE_THROTTLE_THRESHOLD) {
    drive_state = DRIVE;
  }
  if (break_percent > 0 || throttle_percent == 0) {
    drive_state = regen ? BREAK : NEUTRAL;
  }

  // set target current and velocity based on drive state
  // https://tritiumcharging.com/wp-content/uploads/2020/11/TritiumWaveSculptor22_Manual.pdf 18.3
  switch (drive_state) {
    case DRIVE:
      s_target_current = throttle_percent;
      s_target_velocity = TORQUE_CONTROL_VEL;
      break;
    case REVERSE:
      s_target_current = throttle_percent;
      s_target_velocity = -TORQUE_CONTROL_VEL;
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
