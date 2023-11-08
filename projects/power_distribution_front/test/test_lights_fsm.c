#include "lights_fsm.h"
#include "power_distribution_front_getters.h"
#include "task_test_helpers.h"
#include "unity.h"

#define HAZARD_SIGNAL_MSG g_rx_struct.power_info_hazard_state
#define STEERING_ANALOG_SIGNAL_MSG g_rx_struct.steering_info_analog_input

void setup_test(void) {
  log_init();
  init_lights();
}

void teardown_test(void) {}

TEST_IN_TASK
void test_pd_lights_fsm(void) {
  Fsm *f = lights->context;

  // In init state
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(INIT_STATE, f->curr_state);

  // Init State -> Hazard State
  HAZARD_SIGNAL_MSG = HAZARD_ON;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(HAZARD, f->curr_state);

  // Hazard State -> Init State
  HAZARD_SIGNAL_MSG = HAZARD_OFF;
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_OFF;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(INIT_STATE, f->curr_state);

  // Init State -> Left Signal
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_SIGNAL_LEFT;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(LEFT_SIGNAL, f->curr_state);

  // Left Signal ->Init State
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_OFF;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(INIT_STATE, f->curr_state);

  // Init State -> Right Signal
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_SIGNAL_RIGHT;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(RIGHT_SIGNAL, f->curr_state);

  // Right Signal -> Init State
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_OFF;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(INIT_STATE, f->curr_state);

  // Init State -> Left Signal
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_SIGNAL_LEFT;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(LEFT_SIGNAL, f->curr_state);

  // Left Signal -> Right Signal
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_SIGNAL_RIGHT;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(RIGHT_SIGNAL, f->curr_state);

  // Right Signal -> Left Signal
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_SIGNAL_LEFT;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(LEFT_SIGNAL, f->curr_state);

  // Left Signal -> Hazard State
  HAZARD_SIGNAL_MSG = HAZARD_ON;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(HAZARD, f->curr_state);

  // Hazard State -> Init State
  HAZARD_SIGNAL_MSG = HAZARD_OFF;
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_OFF;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(INIT_STATE, f->curr_state);

  // Init State -> Right Signal
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_SIGNAL_RIGHT;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(RIGHT_SIGNAL, f->curr_state);

  // Right Signal -> Hazard State
  HAZARD_SIGNAL_MSG = HAZARD_ON;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(HAZARD, f->curr_state);

  // Hazard State -> Init State
  HAZARD_SIGNAL_MSG = HAZARD_OFF;
  STEERING_ANALOG_SIGNAL_MSG = EE_LIGHT_TYPE_OFF;
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(INIT_STATE, f->curr_state);
}
