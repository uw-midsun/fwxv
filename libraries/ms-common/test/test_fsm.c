#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "fsm.h"
#include "log.h"
#include "task.h"
#include "test_helpers.h"
#include "unity.h"

#define NUM_TEST1_STATES 3
#define NUM_TEST1_EVENTS 3
#define NUM_TEST1_TRANSITIONS 7

FSM(test1, NUM_TEST1_STATES);

// State Definitions for fsm "test1"
typedef enum TestStateId {
  TEST_STATE_0 = 0,
  TEST_STATE_1,
  TEST_STATE_2,
  NUM_TEST_STATES,
} TestStateId;

// Dummy input functions
void prv_state0_input(Fsm *fsm, void *context) {
  return;
}
void prv_state1_input(Fsm *fsm, void *context) {
  return;
}
void prv_state2_input(Fsm *fsm, void *context) {
  return;
}

// Output functions for each state
void prv_state0_output(void *context) {
  uint8_t *transition_count = (uint8_t *)context;
  (*transition_count)++;
}

void prv_state1_output(void *context) {
  uint8_t *transition_count = (uint8_t *)context;
  (*transition_count)++;
}

void prv_state2_output(void *context) {
  uint8_t *transition_count = (uint8_t *)context;
  (*transition_count)++;
}

// Declare states in state list
static FsmState s_test1_state_list[NUM_TEST1_STATES] = {
  STATE(TEST_STATE_0, prv_state0_input, prv_state0_output),
  STATE(TEST_STATE_1, prv_state1_input, prv_state1_output),
  STATE(TEST_STATE_2, prv_state2_input, prv_state2_output),
};

static bool s_test1_transitions[NUM_TEST1_STATES][NUM_TEST1_STATES] = {
  // Transitions for state 0
  TRANSITION(TEST_STATE_0, TEST_STATE_0),
  TRANSITION(TEST_STATE_0, TEST_STATE_1),
  TRANSITION(TEST_STATE_0, TEST_STATE_2),
  // Transitions for state 1
  TRANSITION(TEST_STATE_1, TEST_STATE_2),
  TRANSITION(TEST_STATE_1, TEST_STATE_0),
  // Transitions for state 2
  TRANSITION(TEST_STATE_2, TEST_STATE_2),
  TRANSITION(TEST_STATE_2, TEST_STATE_0),
};

// Start Test Initialization
void setup_test(void) {}
void teardown_test(void) {}

static uint8_t transition_count;
void prv_init_fsm(uint8_t init_state) {
  // Create FSM
  transition_count = 0;
  // Verify FSM initialization
  TEST_ASSERT_OK(_init_fsm(test1_fsm, s_test1_state_list, *s_test1_transitions, init_state,
                           &transition_count));
  TEST_ASSERT_EQUAL_PTR(&transition_count, test1_fsm->context);
  TEST_ASSERT_EQUAL(init_state, test1_fsm->curr_state);
  TEST_ASSERT_EQUAL(NUM_TEST1_STATES, test1_fsm->num_states);
}

// Test table logic
void test_state_table_transitions_success(void) {
  LOG_DEBUG("%ld\n", sizeof(Fsm));
  // Test transitions for state 0
  // No output functions will be executed, as
  prv_init_fsm(TEST_STATE_0);
  // 0 -> 0
  TEST_ASSERT_OK(fsm_transition(test1_fsm, TEST_STATE_0));
  TEST_ASSERT_EQUAL(TEST_STATE_0, test1_fsm->curr_state);
  // 0 -> 1
  TEST_ASSERT_OK(fsm_transition(test1_fsm, TEST_STATE_1));
  TEST_ASSERT_EQUAL(TEST_STATE_1, test1_fsm->curr_state);
  // 1 -> 0
  TEST_ASSERT_OK(fsm_transition(test1_fsm, TEST_STATE_0));
  TEST_ASSERT_EQUAL(TEST_STATE_0, test1_fsm->curr_state);
  // 0 -> 2
  TEST_ASSERT_OK(fsm_transition(test1_fsm, TEST_STATE_2));
  TEST_ASSERT_EQUAL(TEST_STATE_2, test1_fsm->curr_state);

  // Re-initialize in state 1
  prv_init_fsm(TEST_STATE_1);
  TEST_ASSERT_EQUAL(TEST_STATE_1, test1_fsm->curr_state);

  // 1 -> 2
  fsm_transition(test1_fsm, TEST_STATE_2);
  TEST_ASSERT_EQUAL(TEST_STATE_2, test1_fsm->curr_state);
  // 2 -> 2
  fsm_transition(test1_fsm, TEST_STATE_2);
  TEST_ASSERT_EQUAL(TEST_STATE_2, test1_fsm->curr_state);
  // 2 -> 0
  fsm_transition(test1_fsm, TEST_STATE_0);
  TEST_ASSERT_EQUAL(TEST_STATE_0, test1_fsm->curr_state);
}

void test_state_machine_transitions_invalid_id(void) {
  prv_init_fsm(TEST_STATE_0);
  // Event ID greater than max should not transition
  TEST_ASSERT_NOT_OK(fsm_transition(test1_fsm, NUM_TEST_STATES));
  TEST_ASSERT_EQUAL(TEST_STATE_0, test1_fsm->curr_state);

  prv_init_fsm(TEST_STATE_2);
  // If no transition exists for passed state, should not transition
  TEST_ASSERT_NOT_OK(fsm_transition(test1_fsm, TEST_STATE_1));
  TEST_ASSERT_EQUAL(TEST_STATE_2, test1_fsm->curr_state);
}
