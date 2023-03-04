
#include "motor_can.h"

#include <stdint.h>

#include "mcp2515.h"
#include "motor_controller_getters.h"
#include "motor_controller_setters.h"
#include "tasks.h"

#define DRIVER_CONTROL_BASE 0x1

#define MOTOR_CONTROLLER_BASE_L 0x40
#define MOTOR_CONTROLLER_BASE_R 0x80

#define CURRENT_SCALE 100
#define VOLTAGE_SCALE 100
#define VELOCITY_SCALE 100
#define TEMP_SCALE 100

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

static void motor_controller_tx_all() {
  // s_send_message will be false if no can message from center_console and peddle were received
  // since last motor controller transmission
  // TODO: update this with can watchdog
  if (s_send_message != true) {
    return;
  }
  s_send_message = false;

  CanMessage message = {
    .id.raw = DRIVER_CONTROL_BASE + 0x01,
    .dlc = 8,
  };
  memcpy(&message.data_u32[0], &s_target_current, sizeof(s_target_current));
  memcpy(&message.data_u32[0], &s_target_velocity, sizeof(s_target_current));

  mcp2515_transmit(&message);
}

static float uint32_as_float(uint32_t f) {
  union {
    float f;
    uint32_t u;
  } fu = { .f = f };
  return fu.u;
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
        set_motor_controller_vc_mc_current_l(uint32_as_float(msg.data_u32[0]) * CURRENT_SCALE);
        set_motor_controller_vc_mc_voltage_l(uint32_as_float(msg.data_u32[1]) * VOLTAGE_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + BUS_MEASUREMENT:
        set_motor_controller_vc_mc_current_r(uint32_as_float(msg.data_u32[0]) * CURRENT_SCALE);
        set_motor_controller_vc_mc_voltage_r(uint32_as_float(msg.data_u32[1]) * VOLTAGE_SCALE);
        break;

      case MOTOR_CONTROLLER_BASE_L + VEL_MEASUREMENT:
        set_motor_velocity_velocity_l(uint32_as_float(msg.data_u32[0]) * VELOCITY_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + VEL_MEASUREMENT:
        set_motor_velocity_velocity_r(uint32_as_float(msg.data_u32[0]) * VELOCITY_SCALE);
        break;

      case MOTOR_CONTROLLER_BASE_L + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_l(uint32_as_float(msg.data_u32[0]) * TEMP_SCALE);
        set_motor_sink_temps_motor_temp_l(uint32_as_float(msg.data_u32[1]) * TEMP_SCALE);
        break;
      case MOTOR_CONTROLLER_BASE_R + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_r(uint32_as_float(msg.data_u32[0]) * TEMP_SCALE);
        set_motor_sink_temps_motor_temp_r(uint32_as_float(msg.data_u32[1]) * TEMP_SCALE);
        break;
    }
  }
}

void init_motor_controller_can() {
  mcp2515_rx_all = motor_controller_rx_all;
  mcp2515_tx_all = motor_controller_tx_all;
}
