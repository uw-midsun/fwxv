#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "drive_fsm_sequence.h"
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
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
}

// All neutral to [] transitions
void neutral_to_precharge() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DO_PRECHARGE, drive_fsm->curr_state->id);
}

void neutral_to_transmit() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(TRANSMIT, drive_fsm->curr_state->id);
}

// All precharge to [] transitions
void precharge_to_transmit() {
  g_rx_struct.mc_status_precharge_status = 2;  // precharge status is complete
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(TRANSMIT, drive_fsm->curr_state->id);
}

// All transmit to [] transitions
void transmit_to_neutral() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
}

void transmit_to_drive() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DRIVE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(DRIVE, drive_fsm->curr_state->id);
}

void transmit_to_reverse() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(REVERSE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(REVERSE, drive_fsm->curr_state->id);
}

// All drive to [] transitions
void drive_to_transmit() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(TRANSMIT, drive_fsm->curr_state->id);
}

// All reverse to [] transitions
void reverse_to_transmit() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(TRANSMIT, drive_fsm->curr_state->id);
}

TEST_IN_TASK
void test_neutral_to_drive(void) {
  prepare_test();

  // Starting sub test 1 neutral to drive: (Neutral->DoPrecharge->Transmit->Drive)
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();
  precharge_to_transmit();
  transmit_to_drive();

  // Starting sub test 2 drive to neutral: (Drive->Transmit->Neutral)
  notify(drive, NEUTRAL_BUTTON_EVENT);
  drive_to_transmit();
  transmit_to_neutral();

  // Starting sub test 3 neutral to drive: (Neutral->Transmit->Drive) (no precharge)
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 2;  // precharge status is not complete
  neutral_to_transmit();
  transmit_to_drive();

  // Starting sub test 4 drive to neutral: (Drive->Transmit->Neutral) (fault when power state is not
  // main)
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_OFF);
  drive_to_transmit();
  transmit_to_neutral();
}

TEST_IN_TASK
void test_neutral_to_reverse() {
  prepare_test();

  // Starting sub test 1 neutral to reverse: (Neutral->DoPrecharge->Transmit->Reverse)
  notify(drive, REVERSE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = -1;
  g_rx_struct.motor_velocity_velocity_r = -1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();
  precharge_to_transmit();
  transmit_to_reverse();

  // Starting sub test 2 reverse to neutral: (Reverse->Transmit->Neutral)
  notify(drive, NEUTRAL_BUTTON_EVENT);
  reverse_to_transmit();
  transmit_to_neutral();

  // Starting sub test 3 neutral to reverse: (Neutral->Transmit->Reverse) (no precharge)
  notify(drive, REVERSE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = -1;
  g_rx_struct.motor_velocity_velocity_r = -1;
  g_rx_struct.mc_status_precharge_status = 2;  // precharge status is not complete
  neutral_to_transmit();
  transmit_to_reverse();

  // Starting sub test 4 reverse to neutral: (Reverse->Transmit->Neutral) (fault when power state is
  // not main)
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_OFF);
  reverse_to_transmit();
  transmit_to_neutral();
}

TEST_IN_TASK
void test_precharge() {
  prepare_test();

  // Starting sub test 1 neutral to precharge: (Neutral->DoPrecharge)
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();

  // Starting sub test 2 checking if precharge request goes through
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(BEGIN_PRECHARGE_SIGNAL, g_tx_struct.begin_precharge_signal1);

  // Starting sub test 3 checking if precharge times out
  for (int i = 0; i < NUMBER_OF_CYCLES_TO_WAIT - 1; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
  }
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);

  // Starting sub test 4 checking if precharge fails on invalid power state
  notify(drive, DRIVE_BUTTON_EVENT);
  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_MAIN);
  g_rx_struct.motor_velocity_velocity_l = 1;
  g_rx_struct.motor_velocity_velocity_r = 1;
  g_rx_struct.mc_status_precharge_status = 1;  // precharge status is not complete
  neutral_to_precharge();

  fsm_shared_mem_set_power_state(&cc_storage, POWER_FSM_STATE_OFF);
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state->id);
}
