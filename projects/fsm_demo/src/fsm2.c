#include "fsm2.h"

#include "delay.h"
#include "fsm1.h"
#include "log.h"
#include "notify.h"
#include "task.h"

// FSM2 Lags one cycle behind FSM1, so it will transition to state X
// When FSM1 finishes state X
FSM(fsm2, NUM_FSM2_STATES);

static void prv_fsm2_state0_output(void *context) {
  LOG_DEBUG("Transitioned to FSM2 state0\n");
  // Tell fsm1 that we've completed state2
  notify(fsm1, FSM2_STATE_2_CMPL);
}

void prv_fsm2_state0_input(Fsm *fsm, void *context) {
  Fsm *fsm2 = context;
  uint32_t notif;
  // Only transition if FSM1 has finished state 0 and sent notification
  notify_get(&notif);
  if (notify_check_event(&notif, FSM1_STATE_0_CMPL)) {
    fsm_transition(fsm, FSM2_STATE_1);
  }
}

static void prv_fsm2_state1_output(void *context) {
  LOG_DEBUG("Transitioned to FSM2 state1\n");
  // Tell fsm1 that we've completed state1
  notify(fsm1, FSM2_STATE_0_CMPL);
}

static void prv_fsm2_state1_input(Fsm *fsm, void *context) {
  Fsm *fsm2 = context;
  uint32_t notif;
  // Only transition if FSM1 has finished state 1 and sent notification
  notify_get(&notif);
  if (notify_check_event(&notif, FSM1_STATE_1_CMPL)) {
    fsm_transition(fsm, FSM2_STATE_2);
  }
}

static void prv_fsm2_state2_output(void *context) {
  LOG_DEBUG("Transitioned to FSM2 state2\n");
  // Tell fsm1 that we've completed state2
  notify(fsm1, FSM2_STATE_1_CMPL);
}

static void prv_fsm2_state2_input(Fsm *fsm, void *context) {
  Fsm *fsm2 = context;
  uint32_t notif;
  // Only transition if FSM1 has finished state 1 and sent notification
  notify_get(&notif);
  if (notify_check_event(&notif, FSM1_STATE_2_CMPL)) {
    fsm_transition(fsm, FSM2_STATE_0);
  }
}

// Declare states in state list
static FsmState s_test1_state_list[NUM_FSM2_STATES] = {
  STATE(FSM2_STATE_0, prv_fsm2_state0_input, prv_fsm2_state0_output),
  STATE(FSM2_STATE_1, prv_fsm2_state1_input, prv_fsm2_state1_output),
  STATE(FSM2_STATE_2, prv_fsm2_state2_input, prv_fsm2_state2_output),
};

// Declares transition for state machine, must match those in input functions
static FsmTransition s_test1_transitions[NUM_FSM2_TRANSITIONS] = {
  // Transitions for state 0
  TRANSITION(FSM2_STATE_0, FSM2_STATE_1),
  // Transitions for state 1
  TRANSITION(FSM2_STATE_1, FSM2_STATE_2),

  // Transitions for state 2
  TRANSITION(FSM2_STATE_2, FSM2_STATE_0),
};

StatusCode init_fsm2(void) {
  FsmSettings settings = {
    .state_list = s_test1_state_list,
    .transitions = s_test1_transitions,
    .num_transitions = NUM_FSM2_TRANSITIONS,
    .initial_state = FSM2_STATE_0,
  };
  fsm_init(fsm2, settings, NULL);
  return STATUS_CODE_OK;
}
