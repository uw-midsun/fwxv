#include "ltc_afe_fsm.h"

#include "log.h"
#include "ltc_afe_impl.h"
#include "soft_timer.h"

FSM(ltc_afe_fsm, NUM_LTC_AFE_FSM_STATES);

// Global variables for now
static volatile bool trigger_cells = false;
static volatile bool trigger_aux = false;
static volatile bool raise_fault = false;

static void prv_cell_conv_timeout(SoftTimerId timer_id, void *context) {
  // Directly go to the read_cells state after the trigger_cell passes
  trigger_cells = true;
}

static void prv_aux_conv_timeout(SoftTimerId timer_id, void *context) {
  // Directly go to the read_aux state after the trigger_aux passes
  trigger_aux = true;
}

static void prv_afe_idle_output(void *context) {
  raise_fault = false;
  LOG_DEBUG("Transitioned to IDLE state.\n");
}

static void prv_afe_idle_input(Fsm *fsm, void *context) {
  LtcAfeStorage *afe = context;
  // Always transition to trigger_aux
  // We should only ever come back here in case of a fault
  fsm_transition(fsm, LTC_AFE_TRIGGER_CELL_CONV);
}

static void prv_afe_trigger_cell_conv_output(void *context) {
  LtcAfeStorage *afe = context;
  trigger_cells = false;
  StatusCode ret = ltc_afe_impl_trigger_cell_conv(afe);
  if (ret == STATUS_CODE_OK) {
    static SoftTimer s_trigger;
    soft_timer_start(LTC_AFE_FSM_CELL_CONV_DELAY_MS, prv_cell_conv_timeout, &s_trigger);
  } else {
    raise_fault = true;
  }
  LOG_DEBUG("Transitioned to TRIGGER CELLS CONVERSION state.\n");
}

static void prv_afe_trigger_cell_conv_input(Fsm *fsm, void *context) {
  // Transition to read_cells or idle state
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    fsm_transition(fsm, LTC_AFE_IDLE);
  }
  if (trigger_cells) {
    fsm_transition(fsm, LTC_AFE_READ_CELLS);
  }
}

static void prv_afe_read_cells_output(void *context) {
  LtcAfeStorage *afe = context;

  StatusCode ret = ltc_afe_impl_read_cells(afe);

  if (ret == STATUS_CODE_OK) {
    // Take any actions that is appropriate after the read is complete
  } else {
    raise_fault = true;
  }
  LOG_DEBUG("Transitioned to READ CELLS state.\n");
}

static void prv_afe_read_cells_input(Fsm *fsm, void *context) {
  // Transition to trigger aux if no faults have occurred
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    fsm_transition(fsm, LTC_AFE_IDLE);
  }
  prv_extract_read_cell_result(afe->aux_voltages, afe->settings.num_cells,
                               afe->settings.result_context);
  fsm_transition(fsm, LTC_AFE_TRIGGER_AUX_CONV);
}

static void prv_afe_trigger_aux_conv_output(void *context) {
  LtcAfeStorage *afe = context;
  trigger_aux = false;
  uint32_t device_cell = afe->device_cell;
  StatusCode ret = ltc_afe_impl_trigger_aux_conv(afe, device_cell);
  if (ret == STATUS_CODE_OK) {
    afe->aux_index = device_cell;
    static SoftTimer s_trigger;
    soft_timer_start(LTC_AFE_FSM_AUX_CONV_DELAY_MS, prv_aux_conv_timeout, &s_trigger);
  } else {
    raise_fault = true;
  }
  LOG_DEBUG("Transitioned to TRIGGER AUX CONVERSION state.");
}

static void prv_afe_trigger_aux_conv_input(Fsm *fsm, void *context) {
  // Transition to read_aux or idle state
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    fsm_transition(fsm, LTC_AFE_IDLE);
  }
  if (trigger_aux) {
    fsm_transition(fsm, LTC_AFE_READ_AUX);
  }
}

static void prv_afe_read_aux_output(void *context) {
  LtcAfeStorage *afe = context;
  uint16_t device_cell = afe->device_cell;
  StatusCode ret = ltc_afe_impl_read_aux(afe, device_cell);
  if (ret == STATUS_CODE_OK) {
    afe->cell_number++;
  } else {
    raise_fault = true;
  }
  LOG_DEBUG("Transitioned to READ AUX OUTPUT state.");
}

static void prv_afe_read_aux_input(Fsm *fsm, void *context) {
  // Transition to aux_complete, read_aux, trigger_aux_conv, or idle state
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    fsm_transition(fsm, LTC_AFE_IDLE);
  }
  if (afe->cell_number == LTC_AFE_MAX_CELLS_PER_DEVICE) {
    afe->cell_number = 0;
    fsm_transition(fsm, LTC_AFE_AUX_COMPLETE);
  } else {
    fsm_transition(fsm, LTC_AFE_TRIGGER_AUX_CONV);
  }
}

static void prv_afe_aux_complete_output(void *context) {
  LOG_DEBUG("Transitioned to AUX COMPLETE state.");
}

static void prv_afe_aux_complete_input(Fsm *fsm, void *context) {
  // Transition to idle state
  // We can add broadcasting functionality here later (MVP for now)
  LtcAfeStorage *afe = context;
  // 12 aux conversions complete - the array should be fully populated
  prv_extract_cell_result(afe->aux_voltages, afe->settings.num_cells, afe->settings.result_context);
  fsm_transition(fsm, LTC_AFE_TRIGGER_CELL_CONV);
}

// Declare states
static FsmState s_ltc_afe_state_list[NUM_LTC_AFE_FSM_STATES] = {
  STATE(LTC_AFE_IDLE, prv_afe_idle_input, prv_afe_idle_output),
  STATE(LTC_AFE_TRIGGER_CELL_CONV, prv_afe_trigger_cell_conv_input,
        prv_afe_trigger_cell_conv_output),
  STATE(LTC_AFE_READ_CELLS, prv_afe_read_cells_input, prv_afe_read_cells_output),
  STATE(LTC_AFE_TRIGGER_AUX_CONV, prv_afe_trigger_aux_conv_input, prv_afe_trigger_aux_conv_output),
  STATE(LTC_AFE_READ_AUX, prv_afe_read_aux_input, prv_afe_read_aux_output),
  STATE(LTC_AFE_AUX_COMPLETE, prv_afe_aux_complete_input, prv_afe_aux_complete_output),
};

// Declare transitions
static FsmTransition s_ltc_afe_transitions[NUM_LTC_AFE_FSM_TRANSITIONS] = {
  TRANSITION(LTC_AFE_IDLE, LTC_AFE_TRIGGER_CELL_CONV),
  TRANSITION(LTC_AFE_IDLE, LTC_AFE_TRIGGER_AUX_CONV),
  TRANSITION(LTC_AFE_IDLE, LTC_AFE_IDLE),
  TRANSITION(LTC_AFE_TRIGGER_CELL_CONV, LTC_AFE_READ_CELLS),
  TRANSITION(LTC_AFE_TRIGGER_CELL_CONV, LTC_AFE_IDLE),
  TRANSITION(LTC_AFE_READ_CELLS, LTC_AFE_IDLE),
  TRANSITION(LTC_AFE_READ_CELLS, LTC_AFE_TRIGGER_AUX_CONV),
  TRANSITION(LTC_AFE_READ_CELLS, LTC_AFE_READ_CELLS),
  TRANSITION(LTC_AFE_TRIGGER_AUX_CONV, LTC_AFE_READ_AUX),
  TRANSITION(LTC_AFE_TRIGGER_AUX_CONV, LTC_AFE_IDLE),
  TRANSITION(LTC_AFE_READ_AUX, LTC_AFE_AUX_COMPLETE),
  TRANSITION(LTC_AFE_READ_AUX, LTC_AFE_READ_AUX),
  TRANSITION(LTC_AFE_READ_AUX, LTC_AFE_TRIGGER_AUX_CONV),
  TRANSITION(LTC_AFE_READ_AUX, LTC_AFE_IDLE),
  TRANSITION(LTC_AFE_AUX_COMPLETE, LTC_AFE_IDLE)
};

StatusCode init_ltc_afe_fsm(void) {
  FsmSettings settings = {
    .state_list = s_ltc_afe_state_list,
    .transitions = s_ltc_afe_transitions,
    .num_transitions = NUM_LTC_AFE_FSM_TRANSITIONS,
    .initial_state = LTC_AFE_IDLE,
  };
  fsm_init(ltc_afe_fsm, settings, NULL);
  return STATUS_CODE_OK;
}
