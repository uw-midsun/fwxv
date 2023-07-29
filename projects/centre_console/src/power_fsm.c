#include "power_fsm.h"

#include "delay.h"
#include "log.h"
#include "power_fsm_sequence.h"
#include "centre_console_getters.h"
#include "task.h"
#include "cc_hw_defs.h"

static const GpioAddress s_btn_start = CC_BTN_PUSH_START;

FSM(centre_console_power_fsm, NUM_POWER_STATES);

static void prv_power_fsm_off_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  // Start button pressed
  if(gpio_get_state(&s_btn_start, GPIO_STATE_LOW)) {
    // Brake is pressed (any non-zero value)
    if(get_pedal_output_brake_output()) {
      state_context->target_state = POWER_FSM_STATE_AUX;
    } else {
      state_context->target_state = POWER_FSM_STATE_MAIN;
    }
    fsm_transition(fsm, POWER_FSM_CONFIRM_AUX_STATUS);
  }
  return;
}

static void prv_power_fsm_main_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  // Assuming that pressing start again means we're turning off
  if(gpio_get_state(&s_btn_start, GPIO_STATE_LOW)) {
    state_context->target_state = POWER_FSM_STATE_OFF;
    fsm_transition(fsm, POWER_FSM_DISCHARGE_PRECHARGE);
  }
  return;
}

static void prv_power_fsm_aux_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  // If start button && brake are pressed
  if(gpio_get_state(&s_btn_start, GPIO_STATE_LOW) && get_pedal_output_brake_output()) {
    state_context->target_state = POWER_FSM_STATE_MAIN;
    fsm_transition(fsm, POWER_FSM_SEND_PD_BMS);
    // Todo (Bafran): How is off being done? press start again?
  } else if (gpio_get_state(&s_btn_start, GPIO_STATE_LOW)) {
    state_context->target_state = POWER_FSM_STATE_OFF;
    fsm_transition(fsm, POWER_FSM_SEND_PD_BMS);
  }
  return;
}

static void prv_power_fsm_fault_input(Fsm *fsm, void *context) {
  // oh no
  return;
}

static void prv_power_fsm_off_output(void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;
  state_context->latest_state = POWER_FSM_STATE_OFF;
  LOG_DEBUG("CENTRE CONSOLE POWER FSM OFF STATE\n");
}

static void prv_power_fsm_main_output(void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;
  state_context->latest_state = POWER_FSM_STATE_MAIN;
  LOG_DEBUG("CENTRE CONSOLE POWER FSM MAIN STATE\n");
}

static void prv_power_fsm_aux_output(void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;
  state_context->latest_state = POWER_FSM_STATE_AUX;
  LOG_DEBUG("CENTRE CONSOLE POWER FSM AUX STATE\n");
}

static void prv_power_fsm_fault_output(void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;
  state_context->latest_state = POWER_FSM_STATE_FAULT;
  LOG_DEBUG("CENTRE CONSOLE POWER FSM FAULT STATE\n");
}

// Declare states in state lists
static FsmState s_power_fsm_states[NUM_POWER_STATES] = {
  STATE(POWER_FSM_STATE_OFF, prv_power_fsm_off_input, prv_power_fsm_off_output),
  STATE(POWER_FSM_STATE_MAIN, prv_power_fsm_main_input, prv_power_fsm_main_output),
  STATE(POWER_FSM_STATE_AUX, prv_power_fsm_aux_input, prv_power_fsm_aux_output),
  STATE(POWER_FSM_STATE_FAULT, prv_power_fsm_fault_input, prv_power_fsm_fault_output),

  // -> MAIN Sequence
  STATE(POWER_FSM_CONFIRM_AUX_STATUS, prv_power_fsm_confirm_aux_status_input,
        prv_power_fsm_confirm_aux_status_output),
  STATE(POWER_FSM_SEND_PD_BMS, prv_power_fsm_send_pd_bms_input, prv_power_fsm_send_pd_bms_output),
  STATE(POWER_FSM_CONFIRM_BATTERY_STATUS, prv_power_fsm_confirm_battery_status_input,
        prv_power_fsm_confirm_battery_status_output),
  STATE(POWER_FSM_CLOSE_BATTERY_RELAYS, prv_power_fsm_close_battery_relays_input,
        prv_power_fsm_close_battery_relays_output),
  STATE(POWER_FSM_CONFIRM_DC_DC, prv_power_fsm_confirm_dc_dc_input,
        prv_power_fsm_confirm_dc_dc_output),
  STATE(POWER_FSM_TURN_ON_EVERYTHING, prv_power_fsm_turn_on_everything_input,
        prv_power_fsm_turn_on_everything_output),
  STATE(POWER_FSM_POWER_MAIN_COMPLETE, prv_power_fsm_power_main_complete_input,
        prv_power_fsm_power_main_complete_output),

  // -> AUX Sequence
  // Confirm aux status
  // Turn on everything

  // -> OFF Sequence
  STATE(POWER_FSM_DISCHARGE_PRECHARGE, prv_power_fsm_discharge_precharge_input,
        prv_power_fsm_discharge_precharge_output),
  STATE(POWER_FSM_TURN_OFF_EVERYTHING, prv_power_fsm_turn_off_everything_input,
        prv_power_fsm_turn_off_everything_output),
  STATE(POWER_FSM_OPEN_RELAYS, prv_power_fsm_open_relays_input, prv_power_fsm_open_relays_output),
};

// Declares transition for state machine
static FsmTransition s_power_fsm_transitions[NUM_POWER_TRANSITIONS] = {
  // Transitions for OFF state
  TRANSITION(POWER_FSM_STATE_OFF, POWER_FSM_STATE_FAULT),
  // Transitions for MAIN state
  TRANSITION(POWER_FSM_STATE_MAIN, POWER_FSM_STATE_FAULT),
  // Transitions for AUX state
  TRANSITION(POWER_FSM_STATE_AUX, POWER_FSM_STATE_FAULT),

  // Sequence into OFF state
  TRANSITION(POWER_FSM_STATE_MAIN, POWER_FSM_DISCHARGE_PRECHARGE),
  TRANSITION(POWER_FSM_STATE_AUX, POWER_FSM_DISCHARGE_PRECHARGE),
  TRANSITION(POWER_FSM_DISCHARGE_PRECHARGE, POWER_FSM_TURN_OFF_EVERYTHING),
  TRANSITION(POWER_FSM_TURN_OFF_EVERYTHING, POWER_FSM_OPEN_RELAYS),
  TRANSITION(POWER_FSM_OPEN_RELAYS, POWER_FSM_STATE_OFF),

  // Sequence into MAIN state
  TRANSITION(POWER_FSM_STATE_OFF, POWER_FSM_CONFIRM_AUX_STATUS),
  TRANSITION(POWER_FSM_STATE_AUX, POWER_FSM_CONFIRM_AUX_STATUS),
  TRANSITION(POWER_FSM_CONFIRM_AUX_STATUS, POWER_FSM_SEND_PD_BMS),
  TRANSITION(POWER_FSM_SEND_PD_BMS, POWER_FSM_CONFIRM_BATTERY_STATUS),
  TRANSITION(POWER_FSM_CONFIRM_BATTERY_STATUS, POWER_FSM_CLOSE_BATTERY_RELAYS),
  TRANSITION(POWER_FSM_CLOSE_BATTERY_RELAYS, POWER_FSM_CONFIRM_DC_DC),
  TRANSITION(POWER_FSM_CONFIRM_DC_DC, POWER_FSM_TURN_ON_EVERYTHING),
  TRANSITION(POWER_FSM_TURN_ON_EVERYTHING, POWER_FSM_POWER_MAIN_COMPLETE),
  TRANSITION(POWER_FSM_POWER_MAIN_COMPLETE, POWER_FSM_STATE_MAIN),

  // Sequence into AUX state
  TRANSITION(POWER_FSM_STATE_AUX, POWER_FSM_CONFIRM_AUX_STATUS),
  TRANSITION(POWER_FSM_CONFIRM_AUX_STATUS, POWER_FSM_TURN_ON_EVERYTHING),

  // Failures when attempting OFF -> MAIN state
  TRANSITION(POWER_FSM_CONFIRM_AUX_STATUS, POWER_FSM_STATE_OFF),
  TRANSITION(POWER_FSM_SEND_PD_BMS, POWER_FSM_STATE_OFF),
  TRANSITION(POWER_FSM_CONFIRM_BATTERY_STATUS, POWER_FSM_STATE_OFF),
  TRANSITION(POWER_FSM_CLOSE_BATTERY_RELAYS, POWER_FSM_STATE_OFF),
  TRANSITION(POWER_FSM_CONFIRM_DC_DC, POWER_FSM_STATE_OFF),
  TRANSITION(POWER_FSM_TURN_ON_EVERYTHING, POWER_FSM_STATE_OFF),
  TRANSITION(POWER_FSM_POWER_MAIN_COMPLETE, POWER_FSM_STATE_OFF),

  // Failures when attempting AUX -> MAIN state
  TRANSITION(POWER_FSM_CONFIRM_BATTERY_STATUS, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_SEND_PD_BMS, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_CONFIRM_BATTERY_STATUS, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_CLOSE_BATTERY_RELAYS, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_CONFIRM_DC_DC, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_TURN_ON_EVERYTHING, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_POWER_MAIN_COMPLETE, POWER_FSM_STATE_AUX),

  // Failures when attempting OFF -> AUX
  TRANSITION(POWER_FSM_CONFIRM_AUX_STATUS, POWER_FSM_STATE_OFF),
  TRANSITION(POWER_FSM_TURN_ON_EVERYTHING, POWER_FSM_STATE_OFF),
};

StatusCode init_power_fsm(void) {
  // Assuming GPIOs have already been initialized in main
  FsmSettings settings = {
    .state_list = s_power_fsm_states,
    .transitions = s_power_fsm_transitions,
    .num_transitions = NUM_POWER_STATES,
    .initial_state = POWER_FSM_STATE_OFF,
  };
  fsm_init(centre_console_power_fsm, settings, NULL);
  return STATUS_CODE_OK;
}
