
#include "motor_can.h"

#include <stdint.h>

#include "mcp2515.h"
#include "motor_controller_getters.h"
#include "motor_controller_setters.h"
#include "tasks.h"

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
  BRAKE,
  OPD_BRAKE,
} DriveState;

static float s_target_current;
static float s_target_velocity;
static float s_car_velocity_l = 0.0;
static float s_car_velocity_r = 0.0;

static float prv_get_float(uint32_t u) {
  union {
    float f;
    uint32_t u;
  } fu = { .u = u };
  return fu.f;
}

static float prv_one_pedal_drive_current(float throttle_percent, float car_velocity,
                                         DriveState *drive_state) {
  float threshold = car_velocity <= MAX_OPD_SPEED ? car_velocity * COASTING_THERSHOLD_SCALE
                                                  : MAX_COASTING_THRESHOLD;
  if (throttle_percent <= threshold + 0.05 && throttle_percent >= threshold - 0.05) {
    return 0.0;
  }

  if (throttle_percent >= threshold) {
    return (throttle_percent - threshold) / (1 - threshold);
  } else {
    *drive_state = BRAKE;
    return (threshold - throttle_percent) / (threshold);
  }
  LOG_DEBUG("ERROR: One pedal throttle not calculated\n");
  return 0.0;
}

static void prv_update_target_current_velocity() {
  float throttle_percent = prv_get_float(get_pedal_output_throttle_output());
  float brake_percent = prv_get_float(get_pedal_output_brake_output());
  float target_vel = prv_get_float(get_drive_output_target_velocity()) * VEL_TO_RPM_RATIO;
  float car_vel = (s_car_velocity_l + s_car_velocity_r) / 2;

  DriveState drive_state = get_drive_output_drive_state();
  uint8_t regen = get_drive_output_regen_braking();
  bool cruise = get_drive_output_cruise_control();

  // TODO: Update cruise_throttle_threshold so the driver must demand more current than is already
  // being supplied
  if (drive_state == DRIVE && cruise && throttle_percent <= CRUISE_THROTTLE_THRESHOLD) {
    drive_state = CRUISE;
  }
  if (brake_percent > 0 || (throttle_percent == 0 && drive_state != CRUISE)) {
    drive_state = BRAKE;
  }
  if (drive_state == DRIVE || drive_state == REVERSE) {
    throttle_percent = prv_one_pedal_drive_current(throttle_percent, car_vel, &drive_state);
  }

  // set target current and velocity based on drive state
  // https://tritiumcharging.com/wp-content/uploads/2020/11/TritiumWaveSculptor22_Manual.pdf 18.3
  switch (drive_state) {
    case DRIVE:
      s_target_current = throttle_percent;
      s_target_velocity = TORQUE_CONTROL_VEL;
      break;
    case NEUTRAL:
      s_target_current = 0;
      s_target_velocity = 0;
      break;
    case REVERSE:
      s_target_current = throttle_percent;
      s_target_velocity = -TORQUE_CONTROL_VEL;
      break;
    case CRUISE:
      s_target_current = ACCERLATION_FORCE;
      s_target_velocity = target_vel;
      break;
    case BRAKE:  // When braking and regen is off it should be the same as NEUTRAL. regen = 0
      s_target_current = regen < 100 ? regen / 100.0 : throttle_percent;
      s_target_velocity = 0;
      break;
    default:
      // invalid drive state
      return;
  }
}

static void motor_controller_tx_all() {
  // TODO: add can watchdog to shut down motor controller if messages are not received from
  // center console
  prv_update_target_current_velocity();

  CanMessage message = {
    .id.raw = DRIVER_CONTROL_BASE + 0x01,
    .dlc = 8,
  };
  memcpy(&message.data_u32[0], &s_target_current, sizeof(s_target_current));
  memcpy(&message.data_u32[1], &s_target_velocity, sizeof(s_target_velocity));

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
        set_motor_controller_vc_mc_current_l(prv_get_float(msg.data_u32[0]) * CURRENT_SCALE);
        set_motor_controller_vc_mc_voltage_l(prv_get_float(msg.data_u32[1]) * VOLTAGE_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + BUS_MEASUREMENT:
        set_motor_controller_vc_mc_current_r(prv_get_float(msg.data_u32[0]) * CURRENT_SCALE);
        set_motor_controller_vc_mc_voltage_r(prv_get_float(msg.data_u32[1]) * VOLTAGE_SCALE);
        break;

      case MOTOR_CONTROLLER_BASE_L + VEL_MEASUREMENT:
        set_motor_velocity_velocity_l(prv_get_float(msg.data_u32[0]) * VELOCITY_SCALE);
        s_car_velocity_l = prv_get_float(msg.data_u32[1]) * CONVERT_VELOCITY_TO_KPH;
        break;
      case MOTOR_CONTROLLER_BASE_R + VEL_MEASUREMENT:
        set_motor_velocity_velocity_r(prv_get_float(msg.data_u32[0]) * VELOCITY_SCALE);
        s_car_velocity_r = prv_get_float(msg.data_u32[1]) * CONVERT_VELOCITY_TO_KPH;
        break;

      case MOTOR_CONTROLLER_BASE_L + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_l(prv_get_float(msg.data_u32[0]) * TEMP_SCALE);
        set_motor_sink_temps_motor_temp_l(prv_get_float(msg.data_u32[1]) * TEMP_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_r(prv_get_float(msg.data_u32[0]) * TEMP_SCALE);
        set_motor_sink_temps_motor_temp_r(prv_get_float(msg.data_u32[1]) * TEMP_SCALE);
        break;
    }
  }
}

void init_motor_controller_can() {
  mcp2515_rx_all = motor_controller_rx_all;
  mcp2515_tx_all = motor_controller_tx_all;
}
