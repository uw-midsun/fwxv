
#include <stdint.h>

#include "mcp2515.h"
#include "motor_controller_getters.h"
#include "motor_controller_setters.h"
#include "tasks.h"

#define ACCERLATION_FORCE 100
#define DRIVER_CONTROL_BASE 0x1

#define MOTOR_CONTROLLER_BASE_L 0x40
#define MOTOR_CONTROLLER_BASE_R 0x80

typedef enum MotorControllerMessageIds {
  IDENTIFICATION = 0x00,
  STATUS,
  BUS_MEASUREMENT,
  VEL_MEASUREMENT,
  PHASE_CURRENT,
  MOTOR_VOLTAGE,
  MOTOR_CURRENT,
  MOTOR_BACK_EMF,
  RAIL_15V,
  RAIL_3V_9V,
  RESERVED,
  HEAT_SINK_MOTOR_TEMP,
  DSP_BOARD_TEMP,
  RESERVED_2,
  ODOMETER_BUS_AMPHOUR,
  SLIP_SPEED = 0x17,
} MotorControllerMessageIds;

typedef enum DriveState {
  // drive states defined by center console
  DRIVE,
  NEUTRAL,
  REVERSE,
  // extra drive state types used only by mci
  CRUISE,
  BREAK,
} DriveState;

static void motor_controller_tx_all() {
  float current;
  float velocity;

  // verify that can messages from center console, peddal are not stale

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

static void motor_controller_rx_all() {
  CanMessage msg = { 0 };
  while (mcp2515_receive(&msg) == STATUS_CODE_OK) {
    switch (msg.id.raw) {
      case MOTOR_CONTROLLER_BASE_L + STATUS:
        set_motor_status_motor_status_l(msg.data_u32[1]);
        break;
      case MOTOR_CONTROLLER_BASE_R + STATUS:
        set_motor_status_motor_status_r(msg.data_u32[1]);
        break;

      case MOTOR_CONTROLLER_BASE_L + BUS_MEASUREMENT:
        set_motor_controller_vc_mc_current_1(msg.data_u32[0]);
        set_motor_controller_vc_mc_voltage_1(msg.data_u32[1]);
        break;
      case MOTOR_CONTROLLER_BASE_R + BUS_MEASUREMENT:
        set_motor_controller_vc_mc_current_2(msg.data_u32[0]);
        set_motor_controller_vc_mc_voltage_2(msg.data_u32[1]);
        break;

      case MOTOR_CONTROLLER_BASE_L + VEL_MEASUREMENT:
        set_motor_velocity_vehicle_velocity_left(msg.data_u32[0]);
        break;
      case MOTOR_CONTROLLER_BASE_R + VEL_MEASUREMENT:
        set_motor_velocity_vehicle_velocity_right(msg.data_u32[0]);
        break;

      case MOTOR_CONTROLLER_BASE_L + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_sink_temp_l(msg.data_u32[0]);
        set_motor_sink_temps_motor_temp_l(msg.data_u32[1]);
        break;
      case MOTOR_CONTROLLER_BASE_R + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_sink_temp_r(msg.data_u32[0]);
        set_motor_sink_temps_motor_temp_r(msg.data_u32[1]);
        break;
    }
  }
}

void init_motor_controller_can() {
  mcp2515_rx_all = motor_controller_rx_all;
  mcp2515_tx_all = motor_controller_tx_all;
}
