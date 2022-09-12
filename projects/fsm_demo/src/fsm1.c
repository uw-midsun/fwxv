#include "fsm1.h"

#include "delay.h"
#include "fsm2.h"
#include "log.h"
#include "notify.h"
#include "task.h"

FSM(fsm1, NUM_FSM1_STATES);

void prv_fsm1_state0_input(Fsm *fsm, void *context) {
  Fsm *fsm1 = context;
  uint32_t notif;
  // Only transition if FSM1 has finished state 1 and sent notification
  notify_get(&notif);
  if (notify_check_event(&notif, FSM1_STATE_0_CMPL)) {
    fsm_transition(fsm, FSM1_STATE_1);
    LOG_DEBUG("New FSM Cycle\n");
  } else {
    // TODO(mitchellostler): Make this use GPIO_IT notification
    fsm_transition(fsm, FSM1_STATE_1);
  }
}

static void prv_fsm1_state0_output(void *context) {
  LOG_DEBUG("Transitioned to FSM1 state0\n");
  // Tell fsm1 that we've completed state0
  notify(fsm2, FSM1_STATE_0_CMPL);
}

static void prv_fsm1_state1_input(Fsm *fsm, void *context) {
  Fsm *fsm1 = context;
  uint32_t notif;
  // Only transition if FSM1 has finished state 1 and sent notification
  notify_get(&notif);
  if (notify_check_event(&notif, FSM2_STATE_1_CMPL)) {
    fsm_transition(fsm, FSM1_STATE_2);
  }
}

static void prv_fsm1_state1_output(void *context) {
  LOG_DEBUG("Transitioned to FSM1 state1\n");
  // Tell fsm1 that we've completed state0
  notify(fsm2, FSM1_STATE_1_CMPL);
}

static void prv_fsm1_state2_input(Fsm *fsm, void *context) {
  Fsm *fsm1 = context;
  uint32_t notif;
  // Only transition if FSM1 has finished state 1 and sent notification
  notify_get(&notif);
  if (notify_check_event(&notif, FSM1_STATE_2_CMPL)) {
    fsm_transition(fsm, FSM1_STATE_0);
  }
}

static void prv_fsm1_state2_output(void *context) {
  LOG_DEBUG("Transitioned to FSM1 state2\n");
  // Tell fsm1 that we've completed state0
  notify(fsm2, FSM1_STATE_2_CMPL);
}

// Declare states in state list
static FsmState s_test1_state_list[NUM_FSM1_STATES] = {
  STATE(FSM1_STATE_0, prv_fsm1_state0_input, prv_fsm1_state0_output),
  STATE(FSM1_STATE_1, prv_fsm1_state1_input, prv_fsm1_state1_output),
  STATE(FSM1_STATE_2, prv_fsm1_state2_input, prv_fsm1_state2_output),
};

// Declares transition for state machine, must match those in input functions
static FsmTransition s_test1_transitions[NUM_FSM1_TRANSITIONS] = {
  // Transitions for state 0
  TRANSITION(FSM1_STATE_0, FSM1_STATE_0),
  TRANSITION(FSM1_STATE_0, FSM1_STATE_1),
  TRANSITION(FSM1_STATE_0, FSM1_STATE_2),
  // Transitions for state 1
  TRANSITION(FSM1_STATE_1, FSM1_STATE_2),
  TRANSITION(FSM1_STATE_1, FSM1_STATE_0),

  // Transitions for state 2
  TRANSITION(FSM1_STATE_2, FSM1_STATE_2),
  TRANSITION(FSM1_STATE_2, FSM1_STATE_0),
};

StatusCode init_fsm1(void) {
  FsmSettings settings = {
    .state_list = s_test1_state_list,
    .transitions = s_test1_transitions,
    .num_transitions = NUM_FSM1_TRANSITIONS,
    .initial_state = FSM1_STATE_0,
  };
  fsm_init(fsm1, settings, NULL);
  return STATUS_CODE_OK;
}
