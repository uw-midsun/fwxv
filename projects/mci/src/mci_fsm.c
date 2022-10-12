#include "mci_fsm.h"

#include "delay.h"
#include "log.h"
#include "notify.h"
#include "task.h"

FSM(mci_fsm, NUM_MCI_FSM_STATES);

MciFsmStorage s_storage;

void prv_mci_fsm_off_input(Fsm *fsm, void *context) {
  uint32_t notif;
  notify_get(&notif);
  // Can it be possible that both Drive event and Reverse event happen at once?
  // Do we need to check fsm storage to make sure velocity, rpm is 0 in off state?
  if (notify_check_event(&notif, MCI_FSM_GOTO_DRIVE)) {
    fsm_transition(fsm, MCI_FSM_STATE_DRIVE);
  } else if (notify_check_event(&notif, MCI_FSM_GOTO_REVERSE)) {
    fsm_transition(fsm, MCI_FSM_STATE_REVERSE);
  }
}
void prv_mci_fsm_off_output(Fsm *fsm, void *context) {
  LOG_DEBUG("MCI FSM OFF STATE\n");
}

void prv_mci_fsm_drive_input(Fsm *fsm, void *context) {
  uint32_t notif;
  notify_get(&notif);
  if (notify_check_event(&notif, MCI_FSM_GOTO_OFF)) {
    fsm_transition(fsm, MCI_FSM_STATE_OFF);
  }
  // should we check event first or if it valid to transition to event?
  else if (s_storage.velocity == 0 && (&notif, MCI_FSM_GOTO_REVERSE)) {
    fsm_transition(MCI_FSM_STATE_OFF, MCI_FSM_STATE_REVERSE);
  } else if (s_storage.velocity <= CRUISE_MAX_SPEED && s_storage.velocity >= CRUISE_MIN_SPEED &&
             (&notif, MCI_FSM_GOTO_CRUISE)) {
    fsm_transition(fsm, MCI_FSM_STATE_CRUISE);
  }
}
void prv_mci_fsm_drive_output(Fsm *fsm, void *context) {
  LOG_DEBUG("MCI DRIVE CRUISE STATE\n");
}

void prv_mci_fsm_reverse_input(Fsm *fsm, void *context) {
  uint32_t notif;
  notify_get(&notif);
  if (notify_check_event(&notif, MCI_FSM_GOTO_OFF)) {
    fsm_transition(fsm, MCI_FSM_STATE_OFF);
  } else if (s_storage.velocity == 0 && notify_check_event(&notif, MCI_FSM_GOTO_DRIVE)) {
    fsm_transition(fsm, MCI_FSM_STATE_DRIVE);
  }
}
void prv_mci_fsm_reverse_output(Fsm *fsm, void *context) {
  LOG_DEBUG("MCI FSM REVERSE STATE\n");
}

void prv_mci_fsm_cruise_input(Fsm *fsm, void *context) {
  uint32_t notif;
  notify_get(&notif);
  if (notify_check_event(&notif, MCI_FSM_GOTO_OFF)) {
    fsm_transition(fsm, MCI_FSM_STATE_OFF);
  } else if (notify_check_event(&notif, MCI_FSM_GOTO_DRIVE)) {
    fsm_transition(fsm, MCI_FSM_STATE_DRIVE);
  }
}
void prv_mci_fsm_cruise_output(Fsm *fsm, void *context) {
  LOG_DEBUG("MCI FSM CRUISE STATE\n");
}

// Declare states in state list
static FsmState s_test1_state_list[NUM_MCI_FSM_STATES] = {
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
