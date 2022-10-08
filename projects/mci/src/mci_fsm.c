#include "mci_fsm.h"

#include "delay.h"
#include "log.h"
#include "notify.h"
#include "task.h"

FSM(mci_fsm, MCI_FSM_NUM_STATES);

void prv_mci_fsm_off_input(Fsm *fsm, void *context) {
  // check notify event
  // then go to drive, reverse or cruise state based on the event
  // or stay in this state (do nothing)
}
void prv_mci_fsm_off_output(Fsm *fsm, void *context) {}

void prv_mci_fsm_drive_input(Fsm *fsm, void *context) {
  // check notify event
  // if need to go to cruise --> check MIN_CRUISE_SPEED < speed < MAX_CRUISE_SPEED
  // if need to go to reverse --> check speed == 0
  // or go to off or stay in current state
}
void prv_mci_fsm_drive_output(Fsm *fsm, void *context) {}

void prv_mci_fsm_reverse_input(Fsm *fsm, void *context) {}
void prv_mci_fsm_reverse_output(Fsm *fsm, void *context) {}

void prv_mci_fsm_cruise_input(Fsm *fsm, void *context) {}
void prv_mci_fsm_cruise_output(Fsm *fsm, void *context) {}

// Declare states in state list
static FsmState s_test1_state_list[NUM_FSM1_STATES] = {
  STATE(MCI_FSM_STATE_OFF, prv_mci_fsm_off_input, prv_mci_fsm_off_output),
  STATE(MCI_FSM_STATE_DRIVE, prv_mci_fsm_drive_input, prv_mci_fsm_drive_output),
  STATE(MCI_FSM_STATE_REVERSE, prv_mci_fsm_reverse_input, prv_mci_fsm_reverse_output),
  STATE(MCI_FSM_STATE_CRUISE, prv_mci_fsm_cruise_input, prv_mci_fsm_cruise_output),
};

// Declares transition for state machine, must match those in input functions
static FsmTransition s_test1_transitions[NUM_MCI_FSM_TRANSITIONS] = {
  // Transitions for OFF state
  TRANSITION(MCI_FSM_STATE_OFF, MCI_FSM_STATE_DRIVE),
  TRANSITION(MCI_FSM_STATE_OFF, MCI_FSM_STATE_REVERSE),
  // Transitions for DRIVE state
  TRANSITION(MCI_FSM_STATE_DRIVE, MCI_FSM_STATE_OFF),
  TRANSITION(MCI_FSM_STATE_DRIVE, MCI_FSM_STATE_REVERSE),
  TRANSITION(MCI_FSM_STATE_DRIVE, MCI_FSM_STATE_CRUISE),
  // Transitions for REVERSE state
  TRANSITION(MCI_FSM_STATE_REVERSE, MCI_FSM_STATE_OFF),
  TRANSITION(MCI_FSM_STATE_REVERSE, MCI_FSM_STATE_DRIVE),
  // Transitions for CRUISE state
  TRANSITION(MCI_FSM_STATE_CRUISE, MCI_FSM_STATE_OFF),
  TRANSITION(MCI_FSM_STATE_CRUISE, MCI_FSM_STATE_DRIVE),
};

StatusCode init_mci_fsm(void) {
  FsmSettings settings = {
    .state_list = s_test1_state_list,
    .transitions = s_test1_transitions,
    .num_transitions = NUM_MCI_FSM_TRANSITIONS,
    .initial_state = MCI_FSM_STATE_OFF,
  };
  fsm_init(mci_fsm, settings, NULL);
  return STATUS_CODE_OK;
}
