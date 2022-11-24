#include "mci_fsm.h"

#include "delay.h"
#include "log.h"
#include "motor_controller_getters.h"
#include "task.h"

#define get_drive_output() get_drive_output_drive_output()

FSM(mci_fsm, NUM_MCI_FSM_STATES);

static MciFsmStorage s_storage;

static void prv_mci_fsm_off_input(Fsm *fsm, void *context) {
  // TODO(devAdhiraj): added check if power state of MCI is precharged charged
  DriveOutputEvent drive_event = get_drive_output();
  if (drive_event == MCI_FSM_GOTO_DRIVE) {
    fsm_transition(fsm, MCI_FSM_STATE_DRIVE);
  } else if (drive_event == MCI_FSM_GOTO_REVERSE) {
    fsm_transition(fsm, MCI_FSM_STATE_REVERSE);
  }
}

static void prv_mci_fsm_off_output(void *context) {
  LOG_DEBUG("MCI FSM OFF STATE\n");
}

static void prv_mci_fsm_drive_input(Fsm *fsm, void *context) {
  DriveOutputEvent drive_event = get_drive_output();
  if (drive_event == MCI_FSM_GOTO_OFF) {
    fsm_transition(fsm, MCI_FSM_STATE_OFF);
  } else if (drive_event == MCI_FSM_GOTO_REVERSE) {
    if (s_storage.velocity == 0) {
      fsm_transition(fsm, MCI_FSM_STATE_REVERSE);
    } else {
      LOG_DEBUG("reverse requirement not met, didn't transition\n");
    }
  } else if (drive_event == MCI_FSM_GOTO_CRUISE) {
    if (s_storage.velocity <= CRUISE_MAX_SPEED && s_storage.velocity >= CRUISE_MIN_SPEED) {
      fsm_transition(fsm, MCI_FSM_STATE_CRUISE);
    } else {
      LOG_DEBUG("cruise requirement not met, didn't transition\n");
    }
  }
}

static void prv_mci_fsm_drive_output(void *context) {
  LOG_DEBUG("MCI FSM DRIVE STATE\n");
}

static void prv_mci_fsm_reverse_input(Fsm *fsm, void *context) {
  DriveOutputEvent drive_event = get_drive_output();
  if (drive_event == MCI_FSM_GOTO_OFF) {
    fsm_transition(fsm, MCI_FSM_STATE_OFF);
  } else if (drive_event == MCI_FSM_GOTO_DRIVE) {
    if (s_storage.velocity == 0) {
      fsm_transition(fsm, MCI_FSM_STATE_DRIVE);
    } else {
      LOG_DEBUG("drive requirement not met, didn't transition\n");
    }
  }
}

static void prv_mci_fsm_reverse_output(void *context) {
  LOG_DEBUG("MCI FSM REVERSE STATE\n");
}

static void prv_mci_fsm_cruise_input(Fsm *fsm, void *context) {
  DriveOutputEvent drive_event = get_drive_output();
  if (drive_event == MCI_FSM_GOTO_OFF) {
    fsm_transition(fsm, MCI_FSM_STATE_OFF);
  } else if (drive_event == MCI_FSM_GOTO_DRIVE) {
    fsm_transition(fsm, MCI_FSM_STATE_DRIVE);
  }
}
static void prv_mci_fsm_cruise_output(void *context) {
  LOG_DEBUG("MCI FSM CRUISE STATE\n");
}

// Declare states in state list
static FsmState s_test1_state_list[NUM_MCI_FSM_STATES] = {
  STATE(MCI_FSM_STATE_OFF, prv_mci_fsm_off_input, prv_mci_fsm_off_output),
  STATE(MCI_FSM_STATE_DRIVE, prv_mci_fsm_drive_input, prv_mci_fsm_drive_output),
  STATE(MCI_FSM_STATE_REVERSE, prv_mci_fsm_reverse_input, prv_mci_fsm_reverse_output),
  STATE(MCI_FSM_STATE_CRUISE, prv_mci_fsm_cruise_input, prv_mci_fsm_cruise_output),
};

// Declares transition for state machine
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
