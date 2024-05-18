
#include "motor_can.h"

#include <stdint.h>

#include "log.h"
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
  NEUTRAL,
  DRIVE,
  REVERSE,
  // extra drive state types used only by mci
  CRUISE,
  BRAKE,
  OPD_BRAKE
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
  float threshold = 0.0;
  if (car_velocity <= MAX_OPD_SPEED) {
    threshold = car_velocity * COASTING_THRESHOLD_SCALE;
  } else {
    threshold = MAX_COASTING_THRESHOLD;
  }

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
  float throttle_percent = prv_get_float(get_cc_pedal_throttle_output());
  bool brake = get_cc_pedal_brake_output();
  float target_vel = (int)(get_cc_info_target_velocity()) * VEL_TO_RPM_RATIO;
  float car_vel = (s_car_velocity_l + s_car_velocity_r) / 2;

  DriveState drive_state = get_cc_info_drive_state();
  // Regen returns a value btwn 0-100 to represent the max regen we can preform
  // 0 means our cells max voltage is close to 4.2V or regen is off so we should stop regen braking
  // 100 means we are below 4.0V so regen braking is allowed
  float regen = get_drive_output_regen_braking();
  bool cruise = get_cc_info_cruise_control();

   if (drive_state == DRIVE && cruise && throttle_percent <= CRUISE_THROTTLE_THRESHOLD) {
    drive_state = CRUISE;
  }
  if (brake || (throttle_percent == 0 && drive_state != CRUISE)) {
    drive_state = regen ? BRAKE : NEUTRAL;
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
    case REVERSE:
      s_target_current = throttle_percent;
      s_target_velocity = -TORQUE_CONTROL_VEL;
      break;
    case CRUISE:
      s_target_current = ACCERLATION_FORCE;
      s_target_velocity = target_vel;
      break;
    case BRAKE:
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

static inline uint8_t pack_left_shift_u32(uint32_t value, uint8_t shift, uint8_t mask) {
  return (uint8_t)((uint8_t)(value << shift) & mask);
}

static inline uint8_t pack_right_shift_u32(uint32_t value, uint8_t shift, uint8_t mask) {
  return (uint8_t)((uint8_t)(value >> shift) & mask);
}

static void motor_controller_tx_all() {
  // don't send drive command if didn't get centre console's drive output msg
  // if (!get_received_cc_info()) {
  //   LOG_DEBUG("NO drive output\n");
  //   return;
  // }
  // if (!get_pedal_output_brake_output()) return;
  // don't send drive command if not precharged
  if (!g_tx_struct.mc_status_precharge_status) {
    LOG_DEBUG("no precharge\n");
    return;
  }

  prv_update_target_current_velocity();

  CanMessage message = {
    .id.raw = DRIVER_CONTROL_BASE + 0x01,
    .dlc = 8,
  };
  // Very low reading will be ignored
  if (s_target_current < 0.05) {
    s_target_current = 0;
  }
  memcpy(&message.data_u32[0], &s_target_velocity, sizeof(uint32_t));
  memcpy(&message.data_u32[1], &s_target_current, sizeof(uint32_t));

  // LOG_DEBUG("s_target_current: %d\n", (int)(s_target_current * 100));
  // LOG_DEBUG("s_target_velocity: %d\n", (int)(s_target_velocity * 100));

  mcp2515_transmit(&message);
}

static void motor_controller_rx_all() {
  CanMessage msg = { 0 };
  while (mcp2515_receive(&msg) == STATUS_CODE_OK) {
    switch (msg.id.raw) {
      case MOTOR_CONTROLLER_BASE_L + STATUS:
        set_mc_status_error_bitset_l(msg.data_u16[1] >> 1);
        set_mc_status_limit_bitset_l(msg.data_u16[0]);
        break;
      case MOTOR_CONTROLLER_BASE_R + STATUS:
        set_mc_status_error_bitset_r(msg.data_u16[1] >> 1);
        set_mc_status_limit_bitset_r(msg.data_u16[0]);
        break;

      case MOTOR_CONTROLLER_BASE_L + BUS_MEASUREMENT:
        set_motor_controller_vc_mc_current_l(prv_get_float(msg.data_u32[1]) * CURRENT_SCALE);
        set_motor_controller_vc_mc_voltage_l(prv_get_float(msg.data_u32[0]) * VOLTAGE_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + BUS_MEASUREMENT:
        set_motor_controller_vc_mc_current_r(prv_get_float(msg.data_u32[1]) * CURRENT_SCALE);
        set_motor_controller_vc_mc_voltage_r(prv_get_float(msg.data_u32[0]) * VOLTAGE_SCALE);
        break;

      case MOTOR_CONTROLLER_BASE_L + VEL_MEASUREMENT:
        set_motor_velocity_velocity_l(
            (uint16_t)(int16_t)(prv_get_float(msg.data_u32[1]) * VELOCITY_SCALE));
            s_car_velocity_l = prv_get_float(msg.data_u32[1]) * CONVERT_VELOCITY_TO_KPH;
        break;
      case MOTOR_CONTROLLER_BASE_R + VEL_MEASUREMENT:
        set_motor_velocity_velocity_r(
            (uint16_t)(int16_t)(prv_get_float(msg.data_u32[1]) * VELOCITY_SCALE));
            s_car_velocity_r = prv_get_float(msg.data_u32[1]) * CONVERT_VELOCITY_TO_KPH;
        break;

      case MOTOR_CONTROLLER_BASE_L + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_l(prv_get_float(msg.data_u32[1]) * TEMP_SCALE);
        set_motor_sink_temps_motor_temp_l(prv_get_float(msg.data_u32[0]) * TEMP_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_r(prv_get_float(msg.data_u32[1]) * TEMP_SCALE);
        set_motor_sink_temps_motor_temp_r(prv_get_float(msg.data_u32[0]) * TEMP_SCALE);
        break;

      case MOTOR_CONTROLLER_BASE_L + DSP_BOARD_TEMP:
        set_dsp_board_temps_dsp_temp_l(prv_get_float(msg.data_u32[0]) * TEMP_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + DSP_BOARD_TEMP:
        set_dsp_board_temps_dsp_temp_r(prv_get_float(msg.data_u32[0]) * TEMP_SCALE);
        break;
    }
  }
}

void init_motor_controller_can() {
  mcp2515_rx_all = motor_controller_rx_all;
  mcp2515_tx_all = motor_controller_tx_all;
}
