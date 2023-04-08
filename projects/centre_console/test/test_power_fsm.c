#include "task_test_helpers.h"
#include "unity.h"
#include "power_fsm.h"
#include "power_fsm_sequence.h"
#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"

static const GpioAddress s_btn_start = CC_BTN_PUSH_START;
static FsmSettings settings = {
    .state_list = s_power_fsm_states,
    .transitions = s_power_fsm_transitions,
    .num_transitions = NUM_POWER_STATES,
    .initial_state = POWER_FSM_STATE_OFF,
  };

void setup_test(void) {
  log_init();
  gpio_init_pin(&s_btn_start, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
}

void teardown_test(void) {}

TEST_IN_TASK
void test_off_to_main(void) {
  // I'm not sure about this, can we reinit the fsm for each test to start in the correct state?
  settings.initial_state = POWER_FSM_STATE_OFF;
  fsm_init(power_fsm, settings, NULL);

  // Stay in off with no inputs
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_STATE_OFF);

  // Transition to CONFIRM_AUX_STATUS
  gpio_set_state(&s_btn_start, GPIO_STATE_LOW); // Not sure how CC is checking for brake, should this get pedal data as well?
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_CONFIRM_AUX_STATUS);
  gpio_set_state(&s_btn_start, GPIO_STATE_HIGH);

  // Transition to POWER_FSM_SEND_PD_BMS
  g_rx_struct.power_select_status_status = 0x04;
  g_rx_struct.power_select_status_fault = 0;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_SEND_PD_BMS);
  // g_tx_struct.

  // Transition to POWER_FSM_CONFIRM_BATTERY_STATUS
  g_rx_struct.rear_pd_fault_fault_data = 0;
  g_rx_struct.front_pd_fault_fault_data = 0;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_CONFIRM_BATTERY_STATUS);

  // Transition to POWER_FSM_CLOSE_BATTERY_RELAYS
  // g_rx_struct. = 0;
  // g_rx_struct. = 0;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_CLOSE_BATTERY_RELAYS);
  TEST_ASSERT_EQUAL(g_tx_struct.set_relay_states_relay_mask, 1);
  TEST_ASSERT_EQUAL(g_tx_struct.set_relay_states_relay_state, 1);

  // Transition to POWER_FSM_CONFIRM_DC_DC
  // g_rx_struct. = 0;
  // g_rx_struct. = 0;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_CONFIRM_DC_DC);
  
  // Transition to POWER_FSM_TURN_ON_EVERYTHING
  g_rx_struct.power_select_status_status = 0x02;
  g_rx_struct.power_select_status_fault = 0;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_TURN_ON_EVERYTHING);
  // TEST_ASSERT_EQUAL(g_tx_struct., 1);

  // Transition to POWER_FSM_POWER_MAIN_COMPLETE
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_POWER_MAIN_COMPLETE);
  // TEST_ASSERT_EQUAL(g_tx_struct., 1);

  // Transition to POWER_FSM_STATE_MAIN
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_STATE_MAIN);
}

TEST_IN_TASK
void test_off_to_aux(void) {
  settings.initial_state = POWER_FSM_STATE_OFF;
  fsm_init(power_fsm, settings, NULL);

  // Transition to CONFIRM_AUX_STATUS
  gpio_set_state(&s_btn_start, GPIO_STATE_LOW);
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_CONFIRM_AUX_STATUS);
  gpio_set_state(&s_btn_start, GPIO_STATE_HIGH);

  // Transition to POWER_FSM_TURN_ON_EVERYTHING
  g_rx_struct.power_select_status_status = 0x02;
  g_rx_struct.power_select_status_fault = 0;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_TURN_ON_EVERYTHING);
  // TEST_ASSERT_EQUAL(g_tx_struct., 1);
}

TEST_IN_TASK
void test_aux_to_main(void) {
  settings.initial_state = POWER_FSM_STATE_AUX;
  fsm_init(power_fsm, settings, NULL);

  // Transition to POWER_FSM_SEND_PD_BMS
  gpio_set_state(&s_btn_start, GPIO_STATE_LOW); // Not sure how CC is checking for brake, should this get pedal data as well?
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_SEND_PD_BMS);
  gpio_set_state(&s_btn_start, GPIO_STATE_HIGH);

  // If we've gotten this far, the rest of the transitions are already tested in test_off_to_main
}

TEST_IN_TASK
void test_power_to_off(void) {
  // Starting at MAIN, but the behaviour is the same for AUX -> OFF
  settings.initial_state = POWER_FSM_STATE_MAIN;
  fsm_init(power_fsm, settings, NULL);

  // Transition to POWER_FSM_DISCHARGE_PRECHARGE
  gpio_set_state(&s_btn_start, GPIO_STATE_LOW);
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_DISCHARGE_PRECHARGE);
  //TEST_ASSERT_EQUAL(g_tx_struct.discharge_precharge_signal1, 1);

  // Transition to POWER_FSM_TURN_OFF_EVERYTHING
  // g_rx_struct. = 1;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_DISCHARGE_PRECHARGE);

  // Transition to POWER_FSM_OPEN_RELAYS
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_OPEN_RELAYS);
  // TEST_ASSERT_EQUAL(g_tx_struct., 1);
  // TEST_ASSERT_EQUAL(g_tx_struct., 1);

  // Transition to POWER_FSM_STATE_OFF
  // g_rx_struct. = 1;
  // g_rx_struct. = 1;
  fsm_run_cycle(power_fsm);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state->id, POWER_FSM_STATE_OFF);
}
