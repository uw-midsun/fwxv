#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "fsm_shared_mem.h"
#include "power_fsm.h"
#include "task_test_helpers.h"
#include "unity.h"

// run test with command: scons test --platform=x86 --library=centre_console --test=drive_fsm

// buttons defined in drive_fsm.c

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

void prepare_test() {
  init_drive_fsm();

  // test starting neutral state
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
}

// All neutral transitions
void neutral_to_precharge() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DO_PRECHARGE, drive_fsm->curr_state->id);
}

void neutral_to_drive() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DRIVE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(DRIVE, drive_fsm->curr_state->id);
}

void neutral_to_reverse() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(REVERSE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(REVERSE, drive_fsm->curr_state->id);
}

// All precharge transitions
void precharge_to_drive() {
  g_rx_struct.mc_status_precharge_status = 2;  // precharge status is complete
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DRIVE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(DRIVE, drive_fsm->curr_state->id);
}

void precharge_to_reverse() {
  g_rx_struct.mc_status_precharge_status = 2;  // precharge status is complete
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(REVERSE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(REVERSE, drive_fsm->curr_state->id);
}

// All drive transitions
void drive_to_neutral() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
}

// All reverse transitions
void reverse_to_neutral() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
}

// idle tests
void idle_neutral() {
  for (int i = 0; i < 3; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
    TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
    TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
  }
}
void idle_drive() {
  for (int i = 0; i < 3; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
    TEST_ASSERT_EQUAL(DRIVE, g_tx_struct.drive_output_drive_state);
    TEST_ASSERT_EQUAL(DRIVE, drive_fsm->curr_state->id);
  }
}
void idle_reverse() {
  for (int i = 0; i < 3; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
    TEST_ASSERT_EQUAL(REVERSE, g_tx_struct.drive_output_drive_state);
    TEST_ASSERT_EQUAL(REVERSE, drive_fsm->curr_state->id);
  }
}

TEST_IN_TASK
void test_neutral_to_drive(void) {
  LOG_DEBUG("T1: Testing Neutral to Drive\n");
  prepare_test();

  // Starting sub test 1 neutral to drive: (Neutral->DoPrecharge->Drive)
  LOG_DEBUG("T1.1: (Neutral->DoPrecharge->Drive)\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();
  precharge_to_drive();

  // Starting sub test 2 drive idle
  LOG_DEBUG("T1.2: Drive idle\n");
  idle_drive();

  // Starting sub test 3 drive to neutral: (Drive->Neutral)
  LOG_DEBUG("T1.3: (Drive->Neutral)\n");
  notify(drive, NEUTRAL_BUTTON_EVENT);
  drive_to_neutral();

  // Starting sub test 4 neutral idle
  LOG_DEBUG("T1.4: Neutral idle\n");
  idle_neutral();

  // Starting sub test 5 neutral to drive: (Neutral->Drive) (no precharge)
  LOG_DEBUG("T1.5: (Neutral->Drive) (no precharge)\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 2;  // precharge status is not complete
  neutral_to_drive();

  // Starting sub test 6 drive to neutral: (Drive->Neutral) (fault when power state is not
  // main)
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_OFF);
  drive_to_neutral();
}

TEST_IN_TASK
void test_neutral_to_reverse() {
  LOG_DEBUG("T2: Testing Neutral to Reverse\n");
  prepare_test();

  // Starting sub test 1 neutral to reverse: (Neutral->DoPrecharge->Reverse)
  LOG_DEBUG("T2.1: (Neutral->DoPrecharge->Reverse)\n");
  notify(drive, REVERSE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = (uint16_t)-1;
  g_rx_struct.motor_velocity_velocity_r = (uint16_t)-1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();
  precharge_to_reverse();

  // Starting sub test 2 reverse idle
  LOG_DEBUG("T2.2: Reverse idle\n");
  idle_reverse();

  // Starting sub test 3 reverse to neutral: (Reverse->Neutral)
  LOG_DEBUG("T2.3: (Reverse->Neutral)\n");
  notify(drive, NEUTRAL_BUTTON_EVENT);
  reverse_to_neutral();

  // Starting sub test 4 neutral to reverse: (Neutral->Reverse) (no precharge)
  LOG_DEBUG("T2.4: (Neutral->Reverse) (no precharge)\n");
  notify(drive, REVERSE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = (uint16_t)-1;
  g_rx_struct.motor_velocity_velocity_l = (uint16_t)-1;
  g_rx_struct.motor_velocity_velocity_r = (uint16_t)-1;
  g_rx_struct.mc_status_precharge_status = 2;  // precharge status is not complete
  neutral_to_reverse();

  // Starting sub test 5 reverse to neutral: (Reverse->Neutral) (fault when power state is
  // not main)
  LOG_DEBUG("T2.5: (Reverse->Neutral) (fault when power state is not main)\n");
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_OFF);
  reverse_to_neutral();
}

TEST_IN_TASK
void test_precharge() {
  LOG_DEBUG("T3: Testing Precharge\n");
  prepare_test();

  // Starting sub test 1 neutral to precharge: (Neutral->DoPrecharge)
  LOG_DEBUG("T3.1: (Neutral->DoPrecharge)\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();

  // Starting sub test 2 checking if precharge request goes through
  LOG_DEBUG("T3.2: Precharge request goes through\n");
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(BEGIN_PRECHARGE_SIGNAL, g_tx_struct.begin_precharge_signal1);

  // Starting sub test 3 checking if precharge times out and appropriate error code is set
  LOG_DEBUG("T3.3: Precharge times out and appropriate error code is set\n");
  for (int i = 0; i < NUMBER_OF_CYCLES_TO_WAIT - 1; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
  }
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, fsm_shared_mem_get_drive_error_code());

  // Starting sub test 4 checking if error code is reset when going to drive
  LOG_DEBUG("T3.4: Error code is reset when going to drive\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();
  precharge_to_drive();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, fsm_shared_mem_get_drive_error_code());
  notify(drive, NEUTRAL_BUTTON_EVENT);
  drive_to_neutral();

  // Starting sub test 5 checking if precharge fails on invalid power state
  LOG_DEBUG("T3.5: Precharge fails on invalid power state\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();

  fsm_shared_mem_set_power_state(POWER_FSM_STATE_OFF);
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
}
