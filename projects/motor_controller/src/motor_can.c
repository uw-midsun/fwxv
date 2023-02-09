
#include "motor_can.h"

#include <stdint.h>

#include "mcp2515.h"
#include "motor_controller_getters.h"
#include "motor_controller_setters.h"
#include "tasks.h"

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

static void motor_controller_tx_all() {
  // verify that can messages from center console, peddal are not stale
  CanMessage message = {
    .id.raw = DRIVER_CONTROL_BASE + 0x01,
    .data_u32 = { s_target_current, s_target_velocity },
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
        set_motor_controller_vc_mc_current_l(msg.data_u32[0]);
        set_motor_controller_vc_mc_voltage_l(msg.data_u32[1]);
        break;
      case MOTOR_CONTROLLER_BASE_R + BUS_MEASUREMENT:
        set_motor_controller_vc_mc_current_r(msg.data_u32[0]);
        set_motor_controller_vc_mc_voltage_r(msg.data_u32[1]);
        break;

      case MOTOR_CONTROLLER_BASE_L + VEL_MEASUREMENT:
        set_motor_velocity_velocity_l(msg.data_u32[0]);
        break;
      case MOTOR_CONTROLLER_BASE_R + VEL_MEASUREMENT:
        set_motor_velocity_velocity_r(msg.data_u32[0]);
        break;

      case MOTOR_CONTROLLER_BASE_L + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_l(msg.data_u32[0]);
        set_motor_sink_temps_motor_temp_l(msg.data_u32[1]);
        break;
      case MOTOR_CONTROLLER_BASE_R + HEAT_SINK_MOTOR_TEMP:
        set_motor_sink_temps_heatsink_temp_r(msg.data_u32[0]);
        set_motor_sink_temps_motor_temp_r(msg.data_u32[1]);
        break;
    }
  }
}

void init_motor_controller_can() {
  mcp2515_rx_all = motor_controller_rx_all;
  mcp2515_tx_all = motor_controller_tx_all;
}
