#include "lights_fsm.h"
#include "steering_setters.h"
#include "task_test_helpers.h"
#include "unity.h"

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
  TEST_ASSERT_EQUAL(f->curr_state->id, INIT_STATE);

  // Init State -> Hazard State
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_HAZARD);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, HAZARD);

  // Hazard State -> Init State
  set_steering_info_analog_input(EE_LIGHT_TYPE_OFF);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, INIT_STATE);

  // Init State -> Left Signal
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_LEFT);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, LEFT_SIGNAL);

  // Left Signal ->Init State
  set_steering_info_analog_input(EE_LIGHT_TYPE_OFF);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, INIT_STATE);

  // Init State -> Right Signal
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_RIGHT);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, RIGHT_SIGNAL);

  // Right Signal -> Init State
  set_steering_info_analog_input(EE_LIGHT_TYPE_OFF);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, INIT_STATE);

  // Init State -> Left Signal
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_LEFT);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, LEFT_SIGNAL);

  // Left Signal -> Right Signal
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_RIGHT);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, RIGHT_SIGNAL);

  // Right Signal -> Left Signal
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_LEFT);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, LEFT_SIGNAL);

  // Left Signal -> Hazard State
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_HAZARD);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, HAZARD);

  // Hazard State -> Init State
  set_steering_info_analog_input(EE_LIGHT_TYPE_OFF);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, INIT_STATE);

  // Init State -> Right Signal
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_RIGHT);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, RIGHT_SIGNAL);

  // Right Signal -> Hazard State
  set_steering_info_analog_input(EE_LIGHT_TYPE_SIGNAL_HAZARD);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, HAZARD);

  // Hazard State -> Init State
  set_steering_info_analog_input(EE_LIGHT_TYPE_OFF);
  fsm_run_cycle(lights);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(f->curr_state->id, INIT_STATE);
}
