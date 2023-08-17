// Change some syntactical stuff with how the FSM's are declared/used, soft timers
// Remove references to the event queue
#include "ltc_afe_fsm.h"

#include "log.h"
#include "ltc_afe_impl.h"
#include "soft_timer.h"

FSM(ltc_afe_fsm, NUM_LTC_AFE_FSM_STATES);

static void prv_cell_conv_timeout(SoftTimerId timer_id, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  event_raise(afe_events->cell_conv_complete_event, 0);
}

static void prv_aux_conv_timeout(SoftTimerId timer_id, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  event_raise(afe_events->aux_conv_complete_event, afe->aux_index);
}

static void prv_afe_trigger_cell_conv_output(void *context) {
  LOG_DEBUG("Transitioned to TRIGGER CELLS CONVERSION state.\n");
}

static void prv_afe_trigger_cell_conv_input(Fsm *fsm, void *context) {
  LtcAfeStorage *afe = context;
  // LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  StatusCode ret = ltc_afe_impl_trigger_cell_conv(afe);
  // if (ret == STATUS_CODE_OK) {
  //   soft_timer_start_millis(LTC_AFE_FSM_CELL_CONV_DELAY_MS, prv_cell_conv_timeout, afe, NULL);
  // } else {
  //   event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
  //                        LTC_AFE_FSM_FAULT_TRIGGER_CELL_CONV);
  // }
}

static void prv_afe_read_cells_output(void *context) {
  LOG_DEBUG("Transitioned to READ CELLS state.\n")
}

static void prv_afe_read_cells_input(Fsm *fsm, void *context) {
  LtcAfeStorage *afe = context;
  // LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  StatusCode ret = ltc_afe_impl_read_cells(afe);

  // if (status_ok(ret)) {
  //   // Raise the event first in case the user raises a trigger conversion event in the callback
  //   afe->retry_count = 0;
  //   event_raise(afe_events->callback_run_event, 0);

  if (afe->settings.cell_result_cb != NULL) {
    afe->settings.cell_result_cb(afe->cell_voltages, afe->settings.num_cells,
                                 afe->settings.result_context);
  }
  // } else if (afe->retry_count < LTC_AFE_FSM_MAX_RETRY_COUNT) {
  //   afe->retry_count++;
  //   soft_timer_start_millis(LTC_AFE_FSM_CELL_CONV_DELAY_MS, prv_cell_conv_timeout, afe, NULL);
  // } else {
  //   event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
  //                        LTC_AFE_FSM_FAULT_READ_ALL_CELLS);
  // }
}

static void prv_afe_trigger_aux_conv_output(void *context) {
  LOG_DEBUG("Transitioned to TRIGGER AUX CONVERSION state.");
}

static void prv_afe_trigger_aux_conv_input(Fsm *fsm, void *context) {
  LtcAfeStorage *afe = context;
  // LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  // uint32_t device_cell = e->data;
  StatusCode ret = ltc_afe_impl_trigger_aux_conv(afe, device_cell);
  // if (status_ok(ret)) {
  //   afe->aux_index = device_cell;
  //   soft_timer_start_millis(LTC_AFE_FSM_AUX_CONV_DELAY_MS, prv_aux_conv_timeout, afe, NULL);
  // } else {
  //   event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
  //                        LTC_AFE_FSM_FAULT_TRIGGER_AUX_CONV);
  // }
}

static void prv_afe_read_aux_output(void *context) {
  LOG_DEBUG("Transitioned to READ AUX OUTPUT state.");
}

static void prv_afe_read_aux_input(Fsm *fsm, void *context) {
  LtcAfeStorage *afe = context;
  // LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  uint16_t device_cell = e->data;

  StatusCode ret = ltc_afe_impl_read_aux(afe, device_cell);
  // if (status_ok(ret)) {
  //   // Kick-off the next aux conversion
  //   afe->retry_count = 0;
  //   // event_raise(afe_events->trigger_aux_conv_event, device_cell + 1);
  // } else if (afe->retry_count < LTC_AFE_FSM_MAX_RETRY_COUNT) {
  //   // Attempt to retry the read after delaying
  //   afe->retry_count++;
  //   soft_timer_start_millis(LTC_AFE_FSM_AUX_CONV_DELAY_MS, prv_aux_conv_timeout, afe, NULL);
  // } else {
  //   event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
  //                        LTC_AFE_FSM_FAULT_READ_AUX);
  // }
}

static void prv_afe_aux_complete_output(void *context) {}

static void prv_afe_aux_complete_input(Fsm *fsm, void *context) {
  LtcAfeStorage *afe = context;
  // LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  // Raise the event first in case the user raises a trigger conversion event in the callback
  // event_raise(afe_events->callback_run_event, 0);

  // 12 aux conversions complete - the array should be fully populated
  if (afe->settings.aux_result_cb != NULL) {
    afe->settings.aux_result_cb(afe->aux_voltages, afe->settings.num_cells,
                                afe->settings.result_context);
  }
}

// Declare states
static FsmState s_ltc_afe_state_list[NUM_LTC_AFE_FSM_STATES] = {
  STATE(LTC_AFE_IDLE, NULL, NULL),
  STATE(LTC_AFE_TRIGGER_CELL_CONV, prv_afe_trigger_cell_conv_input,
        prv_afe_trigger_cell_conv_output),
  STATE(LTC_AFE_READ_CELLS, prv_afe_read_cells_input, prv_afe_read_cells_output),
  STATE(LTC_AFE_TRIGGER_AUX_CONV, prv_afe_trigger_aux_conv_input, prv_afe_trigger_aux_conv_output),
  STATE(LTC_AFE_READ_AUX, prv_afe_read_aux_input, prv_afe_read_aux_output),
  STATE(LTC_AFE_AUX_COMPLETE, prv_afe_aux_complete_input, prv_afe_aux_complete_output)
};

// Declare transitions
static FsmTransition s_ltc_afe_transitions[NUM_LTC_AFE_FSM_TRANSITIONS] = {
  TRANSITION(LTC_AFE_IDLE, LTC_AFE_TRIGGER_CELL_CONV),
  TRANSITION(LTC_AFE_IDLE, LTC_AFE_TRIGGER_AUX_CONV),
  TRANSITION(LTC_AFE_TRIGGER_CELL_CONV, LTC_AFE_READ_CELLS),
  TRANSITION(LTC_AFE_TRIGGER_CELL_CONV, LTC_AFE_IDLE),
  TRANSITION(LTC_AFE_READ_CELLS, LTC_AFE_IDLE),
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
