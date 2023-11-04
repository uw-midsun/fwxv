#include "power_seq_fsm.h"

FSM(power_seq, NUM_POWER_SEQ_STATES);

static void prv_init_state_input(Fsm *fsm, void *context) {}

static void prv_init_state_output(void *context) {
  LOG_DEBUG("Transitioned to INIT_STATE\n");
}

static void prv_everything_on_input(Fsm *fsm, void *context) {}

static void prv_everything_on_output(void *context) {
  LOG_DEBUG("Transitioned to POWER_MAIN_EVERYTHING_ON\n");
}

static void prv_driver_controls_input(Fsm *fsm, void *context) {}

static void prv_driver_controls_output(void *context) {
  LOG_DEBUG("Transitioned to DRIVER_CONTROLS\n");
}

static void prv_main_operation_input(Fsm *fsm, void *context) {}

static void prv_main_operation_output(void *context) {
  LOG_DEBUG("Transitioned to MAIN_OPERATIONS\n");
}

static void prv_aux_power_input(Fsm *fsm, void *context) {}

static void prv_aux_power_output(void *context) {
  LOG_DEBUG("Transitioned to AUX_POWER\n");
}

// Power Sequence FSM declaration for states and transitions
static FsmState s_power_seq_state_list[NUM_POWER_SEQ_STATES] = {
  STATE(POWER_SEQ_INIT_STATE, prv_init_state_input, prv_init_state_output),
  STATE(POWER_SEQ_EVERYTHING_ON, prv_everything_on_input, prv_everything_on_output),
  STATE(POWER_SEQ_DRIVER_CONTROLS, prv_driver_controls_input, prv_driver_controls_output),
  STATE(POWER_SEQ_MAIN_OPERATION, prv_main_operation_input, prv_main_operation_output),
  STATE(POWER_SEQ_AUX_POWER, prv_aux_power_input, prv_aux_power_output),
};

static bool s_power_seq_transitions[NUM_POWER_SEQ_STATES][NUM_POWER_SEQ_STATES] = {
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
  fsm_init(power_seq, s_power_seq_state_list, s_power_seq_transitions, POWER_SEQ_INIT_STATE, NULL);
  return STATUS_CODE_OK;
}
