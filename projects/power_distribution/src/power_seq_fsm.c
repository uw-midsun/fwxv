#include "power_seq_fsm.h"

#include "power_distribution_front_getters.h"
#include "power_distribution_front_setters.h"

FSM(power_seq, NUM_POWER_SEQ_STATES);

static void prv_init_state_input(Fsm *fsm, void *context) {
  // power main driver BMS signal - removed??

  // else
  fsm_transition(fsm, POWER_SEQ_EVERYTHING_ON);
}

static void prv_init_state_output(void *context) {
  LOG_DEBUG("Transitioned to INIT_STATE");
}

static void prv_everything_on_input(Fsm *fsm, void *context) {
  fsm_transition(fsm, POWER_SEQ_MAIN_OPERATION);
}

static void prv_everything_on_output(void *context) {
  LOG_DEBUG("Transitioned to EVERYTHING_ON");
}

static void prv_driver_controls_input(Fsm *fsm, void *context) {
  if (get_power_off_main_sequence_sequence()) {
    fsm_transition(fsm, POWER_SEQ_INIT_STATE);
  } else if (get_power_on_main_sequence_sequence()) {
    fsm_transition(fsm, POWER_SEQ_MAIN_OPERATION);
  }
}

static void prv_driver_controls_output(void *context) {
  LOG_DEBUG("Transitioned to DRIVER_CONTROLS");
}

static void prv_main_operation_input(Fsm *fsm, void *context) {
  if (get_power_on_aux_sequence_sequence()) {
    fsm_transition(fsm, POWER_SEQ_AUX_POWER);
  } else if (get_power_off_main_sequence_sequence()) {
    fsm_transition(fsm, POWER_SEQ_INIT_STATE);
  } else {
    fsm_transition(fsm, POWER_SEQ_MAIN_OPERATION);  // required?
  }
}

static void prv_main_operation_output(void *context) {
  LOG_DEBUG("Transitioned to MAIN_OPERATIONS");
}

static void prv_aux_power_input(Fsm *fsm, void *context) {
  if (get_power_off_main_sequence_sequence()) {
    fsm_transition(fsm, POWER_SEQ_INIT_STATE);
  }
}

static void prv_aux_power_output(void *context) {
  LOG_DEBUG("Transitioned to AUX_POWER");
}

// Power Sequence FSM declaration for states and transitions
static FsmState s_power_seq_state_list[NUM_POWER_SEQ_STATES] = {
  STATE(POWER_SEQ_INIT_STATE, prv_init_state_input, prv_init_state_output),
  STATE(POWER_SEQ_EVERYTHING_ON, prv_everything_on_input, prv_everything_on_output),
  STATE(POWER_SEQ_DRIVER_CONTROLS, prv_driver_controls_input, prv_driver_controls_output),
  STATE(POWER_SEQ_MAIN_OPERATION, prv_main_operation_input, prv_main_operation_output),
  STATE(POWER_SEQ_AUX_POWER, prv_aux_power_input, prv_aux_power_output),
};

static FsmTransition s_power_seq_transition_list[NUM_POWER_SEQ_TRANSITIONS] = {
  TRANSITION(POWER_SEQ_INIT_STATE, POWER_SEQ_EVERYTHING_ON),
  TRANSITION(POWER_SEQ_INIT_STATE, POWER_SEQ_DRIVER_CONTROLS),
  TRANSITION(POWER_SEQ_EVERYTHING_ON, POWER_SEQ_MAIN_OPERATION),
  TRANSITION(POWER_SEQ_DRIVER_CONTROLS, POWER_SEQ_INIT_STATE),
  TRANSITION(POWER_SEQ_DRIVER_CONTROLS, POWER_SEQ_EVERYTHING_ON),
  TRANSITION(POWER_SEQ_MAIN_OPERATION, POWER_SEQ_INIT_STATE),
  TRANSITION(POWER_SEQ_MAIN_OPERATION, POWER_SEQ_AUX_POWER),
  TRANSITION(POWER_SEQ_MAIN_OPERATION, POWER_SEQ_MAIN_OPERATION),
  TRANSITION(POWER_SEQ_AUX_POWER, POWER_SEQ_INIT_STATE),
};

StatusCode init_power_seq(void) {
  const FsmSettings power_seq_settings = {
    .state_list = s_power_seq_state_list,
    .transitions = s_power_seq_transition_list,
    .num_transitions = NUM_POWER_SEQ_TRANSITIONS,
    .initial_state = POWER_SEQ_INIT_STATE,
  };

  fsm_init(power_seq, power_seq_settings, NULL);
  return STATUS_CODE_OK;
}

// TO DO
// outputs - schematic and depenedent on front/rear pd
// determine between front and rear pd - which one recieves CAN mesasge and transitions
