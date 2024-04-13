// Test motor_can tx (drive commands)

#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_codegen.h"
#include "can_queue.h"
#include "log.h"
#include "mcp2515.h"
#include "misc.h"
#include "motor_can.h"
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

static CanQueue s_mcp2515_tx_queue;

StatusCode TEST_MOCK(mcp2515_hw_transmit)(uint32_t id, bool extended, uint8_t *data, size_t len) {
  CanMessage message = {
    .id.raw = id,
    .extended = extended,
    .dlc = len,
  };
  memcpy(message.data_u8, data, len);
  return can_queue_push(&s_mcp2515_tx_queue, &message);
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

// for test
typedef enum DriveState {
  DRIVE,
  NEUTRAL,
  REVERSE,
} DriveState;

static void assert_drive_command(float current_percent, float velocity_rpm) {
  CanMessage can_message;
  TEST_ASSERT_OK(can_queue_pop(&s_mcp2515_tx_queue, &can_message));

  TEST_ASSERT_EQUAL(DRIVER_CONTROL_BASE + 0x01, can_message.id.raw);
  TEST_ASSERT_EQUAL(prv_get_uint32(current_percent), can_message.data_u32[0]);
  TEST_ASSERT_EQUAL(prv_get_uint32(velocity_rpm), can_message.data_u32[1]);
}

bool initialized = false;
void setup_test(void) {
  // set received drive_output and precharge to true, true for most test
  g_rx_struct.received_drive_output = true;
  g_tx_struct.mc_status_precharge_status = true;

  if (initialized) {
    return;
  }
  initialized = true;

  tasks_init();
  log_init();
  mcp2515_init(&s_mcp2515_storage, &s_mcp2515_settings);
  init_motor_controller_can();
  can_queue_init(&s_mcp2515_tx_queue);
}

void teardown_test(void) {}

void run_motor_controller_cycle() {
  run_mcp2515_rx_cycle();
  wait_tasks(1);
  run_mcp2515_tx_cycle();
  wait_tasks(1);
}

TEST_IN_TASK
void no_drive_command_without_center_console_msg(void) {
  g_rx_struct.received_drive_output = false;
  g_tx_struct.mc_status_precharge_status = true;

  run_motor_controller_cycle();

  CanMessage can_message;
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, can_queue_pop(&s_mcp2515_tx_queue, &can_message));
}

TEST_IN_TASK
void no_drive_command_without_precharge(void) {
  g_rx_struct.received_drive_output = true;
  g_tx_struct.mc_status_precharge_status = false;

  run_motor_controller_cycle();

  CanMessage can_message;
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, can_queue_pop(&s_mcp2515_tx_queue, &can_message));
}

TEST_IN_TASK
void test_reverse(void) {
  g_rx_struct.drive_output_drive_state = REVERSE;
  g_rx_struct.drive_output_cruise_control = false;
  g_rx_struct.drive_output_regen_braking = false;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.3f);

  run_motor_controller_cycle();

  assert_drive_command(0.3f, -TORQUE_CONTROL_VEL);
}

TEST_IN_TASK
void test_neutral(void) {
  g_rx_struct.drive_output_drive_state = NEUTRAL;
  g_rx_struct.drive_output_cruise_control = false;
  g_rx_struct.drive_output_regen_braking = false;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.5f);

  run_motor_controller_cycle();

  assert_drive_command(0.0f, 0.0f);
}

TEST_IN_TASK
void test_neutral_no_pedal(void) {
  g_rx_struct.drive_output_drive_state = DRIVE;
  g_rx_struct.drive_output_cruise_control = false;
  g_rx_struct.drive_output_regen_braking = false;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.0f);

  run_motor_controller_cycle();

  assert_drive_command(0.0f, 0.0f);
}

TEST_IN_TASK
void test_brake(void) {
  g_rx_struct.drive_output_drive_state = REVERSE;
  g_rx_struct.drive_output_cruise_control = false;
  g_rx_struct.drive_output_regen_braking = false;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.5f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.2f);

  run_motor_controller_cycle();

  assert_drive_command(0.0f, 0.0f);
}

TEST_IN_TASK
void test_regen_brake(void) {
  g_rx_struct.drive_output_drive_state = DRIVE;
  g_rx_struct.drive_output_cruise_control = false;
  g_rx_struct.drive_output_regen_braking = true;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.5f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.9f);

  run_motor_controller_cycle();

  assert_drive_command(1.0f, 0.0f);
}

TEST_IN_TASK
void test_drive(void) {
  g_rx_struct.drive_output_cruise_control = false;
  g_rx_struct.drive_output_regen_braking = true;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.3f);

  run_motor_controller_cycle();

  assert_drive_command(0.3f, TORQUE_CONTROL_VEL);
}

TEST_IN_TASK
void test_cruise_drive(void) {
  g_rx_struct.drive_output_cruise_control = true;
  g_rx_struct.drive_output_regen_braking = true;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.0f);

  run_motor_controller_cycle();

  assert_drive_command(1.0f, 10.0f * VEL_TO_RPM_RATIO);
}

TEST_IN_TASK
void test_cruise_braking(void) {
  g_rx_struct.drive_output_cruise_control = true;
  g_rx_struct.drive_output_regen_braking = false;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.1f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.0f);

  run_motor_controller_cycle();

  assert_drive_command(0.0f, 0.0f);
}

TEST_IN_TASK
void test_cruise_regen_braking(void) {
  g_rx_struct.drive_output_cruise_control = true;
  g_rx_struct.drive_output_regen_braking = true;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.1f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.0f);

  run_motor_controller_cycle();

  assert_drive_command(1.0f, 0.0f);
}

TEST_IN_TASK
void test_cruise_throttle(void) {
  g_rx_struct.drive_output_cruise_control = true;
  g_rx_struct.drive_output_regen_braking = true;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.0f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.3f);

  run_motor_controller_cycle();

  assert_drive_command(0.3f, TORQUE_CONTROL_VEL);
}

TEST_IN_TASK
void test_cruise_throttle_and_brake(void) {
  g_rx_struct.drive_output_cruise_control = true;
  g_rx_struct.drive_output_regen_braking = true;
  g_rx_struct.drive_output_target_velocity = prv_get_uint32(10.0f);
  g_rx_struct.pedal_output_brake_output = prv_get_uint32(0.3f);
  g_rx_struct.pedal_output_throttle_output = prv_get_uint32(0.3f);

  run_motor_controller_cycle();

  assert_drive_command(1.0f, 0);
}
