#include "delay.h"
#include "fsm1.h"
#include "fsm2.h"
#include "log.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

// DEMO Test for FSM-based projects

// need to declare a setup test and teardown test function in each test file
// Setup test is called before every test_ function, and teardown after
void setup_test(void) {
  // Initialize log and call fsm1 init
  TEST_ASSERT_OK(init_fsm1());
}
void teardown_test(void) {}

// Here we are testing that init has set things up correctly
// We don't need to do TEST_IN_TASK, since this doesn't need the scheduler
void test_fsm_init(void) {
  // This is a trick for accessing the internal FSM struct
  // fsm_init must be called for this fsm already, otherwise context
  // will not be set
  Fsm *f = fsm1->context;

  // Test that we have initialized into correct state
  TEST_ASSERT_EQUAL(FSM1_STATE_0, f->curr_state->id);
  TEST_ASSERT_EQUAL(NUM_FSM1_STATES, f->num_states);
}

// We will now test the transitions for each state
// We can create a second task
TEST_IN_TASK
void test_fsm_state0_transitions(void) {
  Fsm *f = fsm1->context;
  TEST_ASSERT_EQUAL(FSM1_STATE_0, f->curr_state->id);

  // Need to start fsm2, otherwise notify in fsm1 output will get stuck
  init_fsm2();

  // Fsm1 uses notifications to check whether it needs to change states
  // As this test is running in a different task (test task) we can notify
  // The fsm1 task, and check that it has changed states
  TEST_ASSERT_OK(notify(fsm1, FSM2_STATE_0_CMPL));

  // Run cycle to allow fsm to update
  fsm_run_cycle(fsm1);

  // Wait for fsm task to post that it is finished
  wait_tasks(1);
  TEST_ASSERT_EQUAL(FSM1_STATE_1, f->curr_state->id);
}

// Test all transitions
TEST_IN_TASK
void test_fsm_all_transitions(void) {
  Fsm *f = fsm1->context;
  // Need to start fsm2, otherwise notify in fsm1 output functions will get stuck
  init_fsm2();

  TEST_ASSERT_EQUAL(FSM1_STATE_0, f->curr_state->id);

  TEST_ASSERT_OK(notify(fsm1, FSM2_STATE_2_CMPL));
  fsm_run_cycle(fsm1);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(FSM1_STATE_1, f->curr_state->id);

  TEST_ASSERT_OK(notify(fsm1, FSM2_STATE_0_CMPL));
  fsm_run_cycle(fsm1);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(FSM1_STATE_2, f->curr_state->id);

  TEST_ASSERT_OK(notify(fsm1, FSM2_STATE_1_CMPL));
  fsm_run_cycle(fsm1);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(FSM1_STATE_0, f->curr_state->id);
}
