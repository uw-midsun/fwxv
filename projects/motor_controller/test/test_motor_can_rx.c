// Test motor_can rx (status responses)

#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_codegen.h"
#include "can_queue.h"
#include "delay.h"
#include "fsm.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "misc.h"
#include "motor_can.h"
#include "precharge_control.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

static Mcp2515Storage s_mcp2515_storage = { 0 };
static Mcp2515Settings s_mcp2515_settings = {
  .can_settings = {
    .mode = CAN_CONTINUOUS,
  },
};

StatusCode TEST_MOCK(mcp2515_hw_transmit)(uint32_t id, bool extended, const uint64_t data,
                                          size_t len) {
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mcp2515_hw_init)(const CanQueue *rx_queue, const Mcp2515Settings *settings) {
  return STATUS_CODE_OK;
}

static uint32_t prv_get_uint32(float f) {
  union {
    float f;
    uint32_t u;
  } fu = { .f = f };
  return fu.u;
}

bool initialized = false;
void setup_test(void) {
  if (initialized) return;
  initialized = true;
  tasks_init();
  log_init();
  mcp2515_init(&s_mcp2515_storage, &s_mcp2515_settings);
  init_motor_controller_can();
}

void teardown_test(void) {}

void run_motor_controller_cycle() {
  run_mcp2515_rx_cycle();
  wait_tasks(1);
  run_mcp2515_tx_cycle();
  wait_tasks(1);
}

void push_mc_message(uint32_t id, float data_1, float data_2) {
  CanMessage message = {
    .id.raw = id,
    .data_u32[0] = prv_get_uint32(data_1),
    .data_u32[1] = prv_get_uint32(data_2),
    .dlc = 8,
  };
  TEST_ASSERT_OK(can_queue_push(&s_mcp2515_storage.rx_queue, &message));
}

TEST_IN_TASK
void test_status(void) {
  CanMessage status_l = {
    .id.raw = MOTOR_CONTROLLER_BASE_L + 0x01,
    .data_u8[0] = 1,             // Receive error count
    .data_u8[1] = 2,             // Transmit error count
    .data_u16[1] = 3,            // Active motor
    .data_u16[2] = 0b011001101,  // Error Flags
    .data_u16[3] = 0b110011010,  // Limit Flags
    .dlc = 8,
  };
  can_queue_push(&s_mcp2515_storage.rx_queue, &status_l);
  CanMessage status_r = {
    .id.raw = MOTOR_CONTROLLER_BASE_R + 0x01,
    .data_u8[0] = 1,             // Receive error count
    .data_u8[1] = 2,             // Transmit error count
    .data_u16[1] = 3,            // Active motor
    .data_u16[2] = 0b100110010,  // Error Flags
    .data_u16[3] = 0b001100101,  // Limit Flags
    .dlc = 8,
  };
  can_queue_push(&s_mcp2515_storage.rx_queue, &status_r);

  run_motor_controller_cycle();

  TEST_ASSERT_EQUAL(0b01100110, g_tx_struct.mc_status_error_bitset_l);
  TEST_ASSERT_EQUAL(0b10011010, g_tx_struct.mc_status_limit_bitset_l);
  TEST_ASSERT_EQUAL(0b10011001, g_tx_struct.mc_status_error_bitset_r);
  TEST_ASSERT_EQUAL(0b01100101, g_tx_struct.mc_status_limit_bitset_r);
}

// Bus Measurement (Bus Current: A, Bus Voltage: V)
TEST_IN_TASK
void test_current_voltage(void) {
  push_mc_message(MOTOR_CONTROLLER_BASE_L + 0x02, 0.1f, 0.2f);
  push_mc_message(MOTOR_CONTROLLER_BASE_R + 0x02, 0.3f, 0.4f);

  run_motor_controller_cycle();

  TEST_ASSERT_EQUAL(10, g_tx_struct.motor_controller_vc_mc_current_l);
  TEST_ASSERT_EQUAL(20, g_tx_struct.motor_controller_vc_mc_voltage_l);
  TEST_ASSERT_EQUAL(30, g_tx_struct.motor_controller_vc_mc_current_r);
  TEST_ASSERT_EQUAL(40, g_tx_struct.motor_controller_vc_mc_voltage_r);
}

// Velocity Measurement (Vehicle Velocity: m/s, Motor Velocity: rpm)
TEST_IN_TASK
void test_velocity(void) {
  push_mc_message(MOTOR_CONTROLLER_BASE_L + 0x03, 0.1f, 0.2f);
  push_mc_message(MOTOR_CONTROLLER_BASE_R + 0x03, 0.3f, 0.4f);

  run_motor_controller_cycle();

  TEST_ASSERT_EQUAL(10, g_tx_struct.motor_velocity_velocity_l);
  TEST_ASSERT_EQUAL(30, g_tx_struct.motor_velocity_velocity_r);

  // negative
  push_mc_message(MOTOR_CONTROLLER_BASE_L + 0x03, -0.1f, 0.2f);
  push_mc_message(MOTOR_CONTROLLER_BASE_R + 0x03, -0.3f, 0.4f);

  run_motor_controller_cycle();

  TEST_ASSERT_EQUAL((uint16_t)-10, g_tx_struct.motor_velocity_velocity_l);
  TEST_ASSERT_EQUAL((uint16_t)-30, g_tx_struct.motor_velocity_velocity_r);
}

// Heat-sink & Motor Temperature Measurement (Heat-sink Temp: C, Motor Temp: C)
TEST_IN_TASK
void test_heatsink_motor_temp(void) {
  push_mc_message(MOTOR_CONTROLLER_BASE_L + 0x0B, 0.1f, 0.2f);
  push_mc_message(MOTOR_CONTROLLER_BASE_R + 0x0B, 0.3f, 0.4f);

  run_motor_controller_cycle();

  TEST_ASSERT_EQUAL(10, g_tx_struct.motor_sink_temps_heatsink_temp_l);
  TEST_ASSERT_EQUAL(20, g_tx_struct.motor_sink_temps_motor_temp_l);
  TEST_ASSERT_EQUAL(30, g_tx_struct.motor_sink_temps_heatsink_temp_r);
  TEST_ASSERT_EQUAL(40, g_tx_struct.motor_sink_temps_motor_temp_r);
}

// DSP Board Temperature Measurement (Reserved, DSP Board Temp: C)
TEST_IN_TASK
void test_dsp_temp(void) {
  push_mc_message(MOTOR_CONTROLLER_BASE_L + 0x0C, 0.1f, 0.2f);
  push_mc_message(MOTOR_CONTROLLER_BASE_R + 0x0C, 0.3f, 0.4f);

  run_motor_controller_cycle();

  TEST_ASSERT_EQUAL(20, g_tx_struct.dsp_board_temps_dsp_temp_l);
  TEST_ASSERT_EQUAL(40, g_tx_struct.dsp_board_temps_dsp_temp_r);
}
