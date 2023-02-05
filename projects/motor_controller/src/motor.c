
#include <stdint.h>

#include "mcp2515.h"
#include "motor_controller_getters.h"
#include "tasks.h"

#define ACCERLATION_FORCE 100
#define DRIVER_CONTROL_BASE 0x1

typedef enum DriveState {
  // drive states defined by center console
  DRIVE,
  NEUTRAL,
  REVERSE,
  // extra drive state types used only by mci
  CRUISE,
  BREAK,
} DriveState;

void send_drive_command(DriveState drive_state) {
  float current;
  float velocity;

  uint32_t throttle_percent = get_pedal_throttle_output();
  uint32_t break_percent = get_pedal_brake_output();
  uint32_t target_vel = get_drive_output_target_velocity();
  bool regen = get_regen_braking_state();

  if (drive_state == CRUISE && throttle_percent > 0) {
    drive_state = DRIVE;
  }
  if (break_percent > 0 || throttle_percent == 0) {
    drive_state = regen ? BREAK : NEUTRAL;
  }

  switch (drive_state) {
    case DRIVE:
      current = throttle_percent;
      velocity = 2000;
      break;
    case REVERSE:
      current = throttle_percent;
      velocity = -2000;
      break;
    case CRUISE:
      current = ACCERLATION_FORCE;
      velocity = target_vel;
      break;
    case BREAK:
      current = ACCERLATION_FORCE;
      velocity = 0;
      break;
    case NEUTRAL:
      current = 0;
      velocity = 0;
      break;
    default:
      // invalid drive state
      return;
  }
  CanMessage message = {
    .id.raw = DRIVER_CONTROL_BASE + 0x01,
    .data_u32 = { current, velocity },
    .dlc = 8,
  };

  mcp2515_transmit(&message);
}

void callback() {}
