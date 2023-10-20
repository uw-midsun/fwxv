#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"
#include "power_fsm.h"
#include "power_fsm_can_data.h"
#include "power_fsm_sequence.h"
#include "task_test_helpers.h"
#include "unity.h"

#define PEDAL_PRESSED 0xFF
#define PEDAL_RELEASED 0x00

static const GpioAddress s_btn_start = CC_BTN_PUSH_START;

void setup_test(void) {
  log_init();
  gpio_init_pin(&s_btn_start, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
}

void teardown_test(void) {}

TEST_IN_TASK
void test_off_to_main(void) {
  init_power_fsm(POWER_FSM_STATE_OFF);

  // Stay in off with no inputs
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_STATE_OFF);

  // Transition to CONFIRM_AUX_STATUS
  g_rx_struct.pedal_output_brake_output = PEDAL_PRESSED;
  g_rx_struct.received_pedal_output = true;
  notify(power, START_BUTTON_EVENT);
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_CONFIRM_AUX_STATUS);

  // Transition to POWER_FSM_SEND_PD_BMS
  g_rx_struct.power_select_status_status = AUX_STATUS_BITS;
  g_rx_struct.power_select_status_fault = 0x00;  // Not a fault
  g_rx_struct.received_power_select_status = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_SEND_PD_BMS);

  // Transition to POWER_FSM_CONFIRM_BATTERY_STATUS
  g_rx_struct.rear_pd_fault_fault_data = PD_REAR_FAULT;
  g_rx_struct.front_pd_fault_fault_data = PD_FRONT_FAULT;
  g_rx_struct.received_rear_pd_fault = true;
  g_rx_struct.received_front_pd_fault = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_CONFIRM_BATTERY_STATUS);

  // Transition to POWER_FSM_CLOSE_BATTERY_RELAYS
  g_rx_struct.bps_heartbeat_status = BPS_HEARTBEAT;
  g_rx_struct.received_bps_heartbeat = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_CLOSE_BATTERY_RELAYS);
  TEST_ASSERT_EQUAL(g_tx_struct.set_relay_states_relay_mask, SET_CLOSE_RELAY_STATE_MASK);
  TEST_ASSERT_EQUAL(g_tx_struct.set_relay_states_relay_state, SET_CLOSE_RELAY_STATE_STATE);

  // Transition to POWER_FSM_CONFIRM_DC_DC
  g_rx_struct.battery_relay_state_hv = CLOSE_HV_STATUS;
  g_rx_struct.battery_relay_state_gnd = CLOSE_GND_STATUS;
  g_rx_struct.received_battery_relay_state = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_CONFIRM_DC_DC);

  // Transition to POWER_FSM_TURN_ON_EVERYTHING
  g_rx_struct.power_select_status_status = DCDC_STATUS_BITS;
  g_rx_struct.power_select_status_fault = 0x00;  // No fault
  g_rx_struct.received_power_select_status = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_TURN_ON_EVERYTHING);
  TEST_ASSERT_EQUAL(g_tx_struct.set_power_state_turn_on_everything_notification,
                    SET_TURN_ON_EVERYTHING_NOTIFICATION);

  // Transition to POWER_FSM_POWER_MAIN_COMPLETE
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_POWER_MAIN_COMPLETE);
  TEST_ASSERT_EQUAL(g_tx_struct.ready_to_drive_ready_state, SET_READY_TO_DRIVE);

  // Transition to POWER_FSM_STATE_MAIN
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_STATE_MAIN);
}

TEST_IN_TASK
void test_off_to_aux(void) {
  init_power_fsm(POWER_FSM_STATE_OFF);

  // Transition to CONFIRM_AUX_STATUS
  notify(power, START_BUTTON_EVENT);
  // Brake is not pressed
  g_rx_struct.pedal_output_brake_output = PEDAL_RELEASED;
  g_rx_struct.received_pedal_output = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_CONFIRM_AUX_STATUS);

  // Transition to POWER_FSM_TURN_ON_EVERYTHING
  g_rx_struct.power_select_status_status = AUX_STATUS_BITS;
  g_rx_struct.power_select_status_fault = 0x00;  // No fault
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_TURN_ON_EVERYTHING);
  TEST_ASSERT_EQUAL(g_tx_struct.set_power_state_turn_on_everything_notification,
                    SET_TURN_ON_EVERYTHING_NOTIFICATION);
}

TEST_IN_TASK
void test_aux_to_main(void) {
  init_power_fsm(POWER_FSM_STATE_AUX);

  // Transition to POWER_FSM_SEND_PD_BMS
  notify(power, START_BUTTON_EVENT);
  g_rx_struct.pedal_output_brake_output = PEDAL_PRESSED;
  g_rx_struct.received_pedal_output = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_SEND_PD_BMS);

  // If we've gotten this far, the rest of the transitions are already tested in test_off_to_main
}

TEST_IN_TASK
void test_power_to_off(void) {
  init_power_fsm(POWER_FSM_STATE_MAIN);

  // Transition to POWER_FSM_DISCHARGE_PRECHARGE
  notify(power, START_BUTTON_EVENT);
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_DISCHARGE_PRECHARGE);
  TEST_ASSERT_EQUAL(g_tx_struct.discharge_precharge_signal1, SET_DISCHARGE_PRECHARGE);

  // Transition to POWER_FSM_TURN_OFF_EVERYTHING
  g_rx_struct.precharge_completed_notification = PRECHARGE_COMPLETED_NOTIFCIATION;
  g_rx_struct.received_precharge_completed = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_TURN_OFF_EVERYTHING);

  // Transition to POWER_FSM_OPEN_RELAYS
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_OPEN_RELAYS);
  TEST_ASSERT_EQUAL(g_tx_struct.set_relay_states_relay_mask, SET_OPEN_RELAY_STATE_MASK);
  TEST_ASSERT_EQUAL(g_tx_struct.set_relay_states_relay_state, SET_OPEN_RELAY_STATE_STATE);

  // Transition to POWER_FSM_STATE_OFF
  g_rx_struct.battery_relay_state_hv = OPEN_HV_STATUS;
  g_rx_struct.battery_relay_state_gnd = OPEN_GND_STATUS;
  g_rx_struct.received_battery_relay_state = true;
  fsm_run_cycle(power);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(power_fsm->curr_state, POWER_FSM_STATE_OFF);
}
