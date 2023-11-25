#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "drive_fsm.h"
#include "exported_enums.h"
#include "i2c.h"
#include "task_test_helpers.h"
#include "unity.h"

// run test with command: scons test --platform=x86 --library=centre_console --test=drive_fsm

// buttons defined in drive_fsm.c

void setup_test(void) {
  log_init();
  I2CSettings i2c_setting = {
    .speed = I2C_SPEED_FAST,
  };
  i2c_init(I2C_PORT_1, &i2c_setting);
}

void teardown_test(void) {}

void prepare_test() {
  init_drive_fsm();

  // test starting neutral state
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state);

  g_rx_struct.power_info_pd_fault = STATUS_CODE_OK;
  g_rx_struct.power_info_power_state = EE_POWER_DRIVE_STATE;
  g_rx_struct.received_power_info = true;
}

void neutral_to_drive() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DRIVE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(DRIVE, drive_fsm->curr_state);
}

void neutral_to_reverse() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(REVERSE, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(REVERSE, drive_fsm->curr_state);
}

// All drive transitions
void drive_to_neutral() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state);
}

// All reverse transitions
void reverse_to_neutral() {
  fsm_run_cycle(drive);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
  TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state);
}

// idle tests
void idle_neutral() {
  for (int i = 0; i < 3; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
    TEST_ASSERT_EQUAL(NEUTRAL, g_tx_struct.drive_output_drive_state);
    TEST_ASSERT_EQUAL(NEUTRAL, drive_fsm->curr_state);
  }
}
void idle_drive() {
  for (int i = 0; i < 3; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
    TEST_ASSERT_EQUAL(DRIVE, g_tx_struct.drive_output_drive_state);
    TEST_ASSERT_EQUAL(DRIVE, drive_fsm->curr_state);
  }
}
void idle_reverse() {
  for (int i = 0; i < 3; i++) {
    fsm_run_cycle(drive);
    wait_tasks(1);
    TEST_ASSERT_EQUAL(REVERSE, g_tx_struct.drive_output_drive_state);
    TEST_ASSERT_EQUAL(REVERSE, drive_fsm->curr_state);
  }
}

TEST_IN_TASK
void test_neutral_to_drive(void) {
  LOG_DEBUG("T1: Testing Neutral to Drive\n");
  prepare_test();

  // Starting sub test 1 neutral to dive: (Neutral->Drive)
  LOG_DEBUG("T1.1: (Neutral->Drive)\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  neutral_to_drive();

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

  // Starting sub test 5 drive to neutral: (Drive->Neutral) (fault when power error state
  // is not STATUS_CODE_OK)
  LOG_DEBUG("T1.5: (Drive->Neutral) (fault when power error state is not STATUS_CODE_OK)\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  neutral_to_drive();
  g_rx_struct.power_info_pd_fault = STATUS_CODE_UNKNOWN;  // set test value
  drive_to_neutral();
  g_rx_struct.power_info_pd_fault = STATUS_CODE_OK;  // reset test value for next test

  // Starting sub test 6 drive to neutral: (Drive->Neutral) (fault when power state is
  // not main)
  LOG_DEBUG("T1.6: (Drive->Neutral) (fault when power state is not main)\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  neutral_to_drive();
  g_rx_struct.power_info_power_state = EE_POWER_OFF_STATE;  // set test value 
  drive_to_neutral();
  g_rx_struct.power_info_power_state =
      EE_POWER_DRIVE_STATE;  // reset test value for next test 

  // Starting sub test 7 drive to neutral: (Drive->Neutral) (fault when we don't
  // receive power state message)
  LOG_DEBUG("T1.7: (Drive->Neutral) (fault when we don't receive power state message)\n");
  notify(drive, DRIVE_BUTTON_EVENT);
  neutral_to_drive();
  g_rx_struct.received_power_info = false;  // set test value
  fsm_run_cycle(drive);
  wait_tasks(1);
  fsm_run_cycle(drive);
  wait_tasks(1);
  fsm_run_cycle(drive);
  wait_tasks(1);
  drive_to_neutral();
  g_rx_struct.received_power_info = true;  // reset test value for next test
}

TEST_IN_TASK
void test_neutral_to_reverse() {
  LOG_DEBUG("T2: Testing Neutral to Reverse\n");
  prepare_test();

  // Starting sub test 1 neutral to reverse: (Neutral->Reverse)
  LOG_DEBUG("T2.1: (Neutral->Reverse)\n");
  notify(drive, REVERSE_BUTTON_EVENT);
  neutral_to_reverse();

  // Starting sub test 2 reverse idle
  LOG_DEBUG("T2.2: Reverse idle\n");
  idle_reverse();

  // Starting sub test 3 reverse to neutral: (Reverse->Neutral)
  LOG_DEBUG("T2.3: (Reverse->Neutral)\n");
  notify(drive, NEUTRAL_BUTTON_EVENT);
  reverse_to_neutral();

  // Starting sub test 4 reverse to neutral: (Reverse->Neutral) (fault when power error state
  // is not STATUS_CODE_OK)
  LOG_DEBUG("T2.4: (Reverse->Neutral) (fault when power error state is not STATUS_CODE_OK)\n");
  notify(drive, REVERSE_BUTTON_EVENT);
  neutral_to_reverse();
  g_rx_struct.power_info_pd_fault = STATUS_CODE_UNKNOWN;  // set test value
  reverse_to_neutral();
  g_rx_struct.power_info_pd_fault = STATUS_CODE_OK;  // reset test value for next test

  // Starting sub test 5 reverse to neutral: (Reverse->Neutral) (fault when power state is
  // not main)
  LOG_DEBUG("T2.5: (Reverse->Neutral) (fault when power state is not main)\n");
  notify(drive, REVERSE_BUTTON_EVENT);
  neutral_to_reverse();
  g_rx_struct.power_info_power_state = EE_POWER_OFF_STATE;  // set test value 
  reverse_to_neutral();
  g_rx_struct.power_info_power_state =
      EE_POWER_DRIVE_STATE;  // reset test value for next test 

  // Starting sub test 6 reverse to neutral: (Reverse->Neutral) (fault when we don't
  // receive power state message)
  LOG_DEBUG("T2.6: (Reverse->Neutral) (fault when we don't receive power state message)\n");
  notify(drive, REVERSE_BUTTON_EVENT);
  neutral_to_reverse();
  g_rx_struct.received_power_info = false;  // set test value
  fsm_run_cycle(drive);
  wait_tasks(1);
  fsm_run_cycle(drive);
  wait_tasks(1);
  fsm_run_cycle(drive);
  wait_tasks(1);
  reverse_to_neutral();
  g_rx_struct.received_power_info = true;  // reset test value for next test
}
