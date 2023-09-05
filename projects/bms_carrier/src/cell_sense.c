#include "cell_sense.h"

#include <string.h>

#include "current_sense.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "log.h"
#include "ltc_afe_impl.h"
#include "passive_balance.h"
#include "soft_timer.h"
#include "status.h"

// Global variables for now
// For now, transition to IDLE
// TODO: Create some kind of fault mechanism if driver function fails
bool raise_fault = false;

StatusCode prv_ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  status_ok_or_return(ltc_afe_impl_init(afe, settings));
  return prv_init_ltc_afe_fsm(&afe->fsm, afe);
}

FSM(ltc_afe_fsm, NUM_LTC_AFE_FSM_STATES);

static CellSenseStorage s_storage = { 0 };

static void prv_extract_cell_result(uint16_t *result_arr, size_t len, void *context) {
  memcpy(s_storage.readings->voltages, result_arr, sizeof(s_storage.readings->voltages));

  bool fault = false;
  for (size_t i = 0; i < len; i++) {
    // s_storage.total_voltage += s_storage.readings->voltages[i];
    if (s_storage.readings->voltages[i] < s_storage.settings.undervoltage_dmv ||
        s_storage.readings->voltages[i] > s_storage.settings.overvoltage_dmv) {
      fault = true;
    }
  }

  // Balance cells if needed
  passive_balance(s_storage.readings->voltages, len, s_storage.afe);

  // TODO: Find what we should do when encountering faulty results
  if (fault) {
    fault_bps_set(EE_BPS_STATE_FAULT_AFE_CELL);
  } else {
    fault_bps_clear(EE_BPS_STATE_FAULT_AFE_CELL);
  }
}

static void prv_extract_aux_result(uint16_t *result_arr, size_t len, void *context) {
  memcpy(s_storage.readings->temps, result_arr, sizeof(s_storage.readings->temps));

  uint16_t threshold = s_storage.settings.discharge_overtemp_dmv;
  if (current_sense_is_charging()) threshold = s_storage.settings.charge_overtemp_dmv;

  for (size_t i = 0; i < len; ++i) {
    if (s_storage.readings->temps[i] > threshold) {
      // TODO: Find what we should do when encountering faulty results
      fault_bps_set(EE_BPS_STATE_FAULT_AFE_TEMP);
      return;
    }
  }

  fault_bps_clear(EE_BPS_STATE_FAULT_AFE_TEMP);
}

static void prv_afe_idle_output(void *context) {
  if (raise_fault) {
    // TODO: Implement some kind of error handling
  }
  LOG_DEBUG("Transitioned to IDLE state.\n");
}

static void prv_afe_idle_input(Fsm *fsm, void *context) {
  LtcAfeStorage *afe = context;
  // Always transition to trigger_aux. Might remove this state tbh
  if (!raise_fault) {
    fsm_transition(fsm, LTC_AFE_TRIGGER_CELL_CONV);
  }
}

static void prv_afe_trigger_cell_conv_output(void *context) {
  LtcAfeStorage *afe = context;
  StatusCode ret = ltc_afe_impl_trigger_cell_conv(afe);
  if (ret != STATUS_CODE_OK) {
    raise_fault = true;
  }
  afe->time_elapsed = 1000 * (xTaskGetTickCount() / 1024);
  LOG_DEBUG("Transitioned to TRIGGER CELLS CONVERSION state.\n");
}

static void prv_afe_trigger_cell_conv_input(Fsm *fsm, void *context) {
  // Transition to read_cells or idle state
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    fsm_transition(fsm, LTC_AFE_IDLE);
  }

  uint16_t current_time = 1000 * (xTaskGetTickCount() / 1024);
  if (current_time - afe->time_elapsed > LTC_AFE_FSM_CELL_CONV_DELAY_MS) {
    afe->retry_count = 0;
    fsm_transition(fsm, LTC_AFE_READ_CELLS);
  }
}

static void prv_afe_read_cells_output(void *context) {
  LtcAfeStorage *afe = context;

  StatusCode ret = ltc_afe_impl_read_cells(afe);

  if (ret != STATUS_CODE_OK) {
    raise_fault = true;
  }
  LOG_DEBUG("Transitioned to READ CELLS state.\n");
}

static void prv_afe_read_cells_input(Fsm *fsm, void *context) {
  // Transition to trigger_aux if no faults have occurred
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    if (afe->retry_count < LTC_AFE_FSM_MAX_RETRY_COUNT) {
      afe->retry_count++;
      fsm_transition(fsm, LTC_AFE_READ_CELLS);
    } else {
      fsm_transition(fsm, LTC_AFE_IDLE);
    }
  }
  prv_extract_cell_result(afe->cell_voltages, afe->settings.num_cells,
                          afe->settings.result_context);
  fsm_transition(fsm, LTC_AFE_TRIGGER_AUX_CONV);
}

static void prv_afe_trigger_aux_conv_output(void *context) {
  LtcAfeStorage *afe = context;
  uint32_t device_cell = afe->device_cell;
  StatusCode ret = ltc_afe_impl_trigger_aux_conv(afe, device_cell);
  if (ret == STATUS_CODE_OK) {
    afe->aux_index = device_cell;
  } else {
    raise_fault = true;
  }
  afe->time_elapsed = 1000 * (xTaskGetTickCount() / 1024);
  LOG_DEBUG("Transitioned to TRIGGER AUX CONVERSION state.");
}

static void prv_afe_trigger_aux_conv_input(Fsm *fsm, void *context) {
  // Transition to read_aux or idle state
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    fsm_transition(fsm, LTC_AFE_IDLE);
  }

  uint16_t current_time = 1000 * (xTaskGetTickCount() / 1024);
  if (current_time - afe->time_elapsed > LTC_AFE_FSM_AUX_CONV_DELAY_MS) {
    afe->retry_count = 0;
    fsm_transition(fsm, LTC_AFE_READ_AUX);
  }
}

static void prv_afe_read_aux_output(void *context) {
  LtcAfeStorage *afe = context;
  StatusCode ret = ltc_afe_impl_read_aux(afe, afe->device_cell);
  if (ret == STATUS_CODE_OK) {
    afe->device_cell++;
  } else {
    raise_fault = true;
  }
  LOG_DEBUG("Transitioned to READ AUX OUTPUT state.");
}

static void prv_afe_read_aux_input(Fsm *fsm, void *context) {
  // Transition to aux_complete, read_aux, trigger_aux_conv, or idle state
  LtcAfeStorage *afe = context;
  if (raise_fault) {
    if (afe->retry_count < LTC_AFE_FSM_MAX_RETRY_COUNT) {
      afe->retry_count++;
      fsm_transition(fsm, LTC_AFE_READ_AUX);
    } else {
      fsm_transition(fsm, LTC_AFE_IDLE);
    }
  }
  if (afe->device_cell == LTC_AFE_MAX_CELLS_PER_DEVICE) {
    afe->device_cell = 0;
    fsm_transition(fsm, LTC_AFE_AUX_COMPLETE);
  } else {
    fsm_transition(fsm, LTC_AFE_TRIGGER_AUX_CONV);
  }
}

static void prv_afe_aux_complete_output(void *context) {
  LOG_DEBUG("Transitioned to AUX COMPLETE state.");
}

static void prv_afe_aux_complete_input(Fsm *fsm, void *context) {
  // We can add broadcasting functionality here later (MVP for now)
  LtcAfeStorage *afe = context;
  // 12 aux conversions complete - the array should be fully populated
  prv_extract_aux_result(afe->aux_voltages, afe->settings.num_cells, afe->settings.result_context);
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
  TRANSITION(LTC_AFE_AUX_COMPLETE, LTC_AFE_TRIGGER_CELL_CONV),
};

StatusCode prv_init_ltc_afe_fsm(void) {
  FsmSettings settings = {
    .state_list = s_ltc_afe_state_list,
    .transitions = s_ltc_afe_transitions,
    .num_transitions = NUM_LTC_AFE_FSM_TRANSITIONS,
    .initial_state = LTC_AFE_IDLE,
  };
  fsm_init(ltc_afe_fsm, settings, NULL);
  return STATUS_CODE_OK;
}

StatusCode cell_sense_init(const CellSenseSettings *settings, AfeReadings *afe_readings,
                           LtcAfeStorage *afe) {
  s_storage.afe = afe;
  s_storage.readings = afe_readings;
  memset(afe_readings, 0, sizeof(AfeReadings));
  memcpy(&s_storage.settings, settings, sizeof(CellSenseSettings));
  return prv_ltc_afe_init(afe, settings);
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return ltc_afe_impl_toggle_cell_discharge(afe, cell, discharge);
}
