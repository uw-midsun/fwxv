#include "power_fsm.h"

#include "cc_hw_defs.h"
#include "centre_console_getters.h"
#include "delay.h"
#include "fsm_shared_mem.h"
#include "gpio_it.h"
#include "log.h"
#include "power_fsm_sequence.h"
#include "task.h"

// Todo(Bafran): Update to real value
#define BRAKE_THRES 0x0F

static const GpioAddress s_btn_start = CC_BTN_PUSH_START;
static FSMStorage shared_mem;

PowerFsmContext power_context = { 0 };

FSM(power, NUM_POWER_STATES);

static void prv_power_fsm_off_input(Fsm *fsm, void *context) {
  // Start button pressed
  uint32_t notifications = 0;
  notify_get(&notifications);
  if (notifications & (1 << START_BUTTON_EVENT)) {
    // Brake is pressed (any non-zero value)
    if (get_pedal_output_brake_output()) {
      power_context.target_state = POWER_FSM_STATE_MAIN;
    } else {
      power_context.target_state = POWER_FSM_STATE_AUX;
    }
    fsm_transition(fsm, POWER_FSM_CONFIRM_AUX_STATUS);
  }
  return;
}

static void prv_power_fsm_main_input(Fsm *fsm, void *context) {
  uint32_t notifications = 0;
  notify_get(&notifications);
  // Assuming that pressing start again means we're turning off
  if (notifications & (1 << START_BUTTON_EVENT)) {
    power_context.target_state = POWER_FSM_STATE_OFF;
    fsm_transition(fsm, POWER_FSM_DISCHARGE_PRECHARGE);
  }
  return;
}

static void prv_power_fsm_aux_input(Fsm *fsm, void *context) {
  uint32_t notifications = 0;
  notify_get(&notifications);
  // If start button && brake are pressed
  if (notifications & (1 << START_BUTTON_EVENT) && get_pedal_output_brake_output()) {
    power_context.target_state = POWER_FSM_STATE_MAIN;
    fsm_transition(fsm, POWER_FSM_SEND_PD_BMS);
  } else if (notifications & (1 << START_BUTTON_EVENT)) {
    power_context.target_state = POWER_FSM_STATE_OFF;
    fsm_transition(fsm, POWER_FSM_DISCHARGE_PRECHARGE);
  }
  return;
}

static void prv_power_fsm_fault_input(Fsm *fsm, void *context) {
  return;
}

static void prv_power_fsm_off_output(void *context) {
  power_context.latest_state = POWER_FSM_STATE_OFF;
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_OFF);
  fsm_shared_mem_set_power_error_code(STATUS_CODE_OK);
  LOG_DEBUG("CENTRE CONSOLE POWER FSM OFF STATE\n");
}

static void prv_power_fsm_main_output(void *context) {
  power_context.latest_state = POWER_FSM_STATE_MAIN;
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_MAIN);
  fsm_shared_mem_set_power_error_code(STATUS_CODE_OK);
  LOG_DEBUG("CENTRE CONSOLE POWER FSM MAIN STATE\n");
}

static void prv_power_fsm_aux_output(void *context) {
  power_context.latest_state = POWER_FSM_STATE_AUX;
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_AUX);
  fsm_shared_mem_set_power_error_code(STATUS_CODE_OK);
  LOG_DEBUG("CENTRE CONSOLE POWER FSM AUX STATE\n");
}

static void prv_power_fsm_fault_output(void *context) {
  power_context.latest_state = POWER_FSM_STATE_FAULT;
  fsm_shared_mem_set_power_state(POWER_FSM_STATE_FAULT);
  fsm_shared_mem_set_power_error_code(STATUS_CODE_OK);
  LOG_DEBUG("CENTRE CONSOLE POWER FSM FAULT STATE\n");
}

// Declare states in state lists
static FsmState s_power_fsm_states[NUM_POWER_STATES] = {
  STATE(POWER_FSM_STATE_OFF, prv_power_fsm_off_input, prv_power_fsm_off_output),
  STATE(POWER_FSM_STATE_MAIN, prv_power_fsm_main_input, prv_power_fsm_main_output),
  STATE(POWER_FSM_STATE_AUX, prv_power_fsm_aux_input, prv_power_fsm_aux_output),
  STATE(POWER_FSM_STATE_FAULT, prv_power_fsm_fault_input, prv_power_fsm_fault_output),

  // -> MAIN Sequence
  STATE(POWER_FSM_CONFIRM_AUX_STATUS, power_fsm_confirm_aux_status_input,
        power_fsm_confirm_aux_status_output),
  STATE(POWER_FSM_SEND_PD_BMS, power_fsm_send_pd_bms_input, power_fsm_send_pd_bms_output),
  STATE(POWER_FSM_CONFIRM_BATTERY_STATUS, power_fsm_confirm_battery_status_input,
        power_fsm_confirm_battery_status_output),
  STATE(POWER_FSM_CLOSE_BATTERY_RELAYS, power_fsm_close_battery_relays_input,
        power_fsm_close_battery_relays_output),
  STATE(POWER_FSM_CONFIRM_DC_DC, power_fsm_confirm_dc_dc_input, power_fsm_confirm_dc_dc_output),
  STATE(POWER_FSM_TURN_ON_EVERYTHING, power_fsm_turn_on_everything_input,
        power_fsm_turn_on_everything_output),
  STATE(POWER_FSM_POWER_MAIN_COMPLETE, power_fsm_power_main_complete_input,
        power_fsm_power_main_complete_output),

  // -> AUX Sequence
  // Confirm aux status
  // Turn on everything

  // -> OFF Sequence
  STATE(POWER_FSM_DISCHARGE_PRECHARGE, power_fsm_discharge_precharge_input,
        power_fsm_discharge_precharge_output),
  STATE(POWER_FSM_TURN_OFF_EVERYTHING, power_fsm_turn_off_everything_input,
        power_fsm_turn_off_everything_output),
  STATE(POWER_FSM_OPEN_RELAYS, power_fsm_open_relays_input, power_fsm_open_relays_output),
};

static bool s_power_transitions[NUM_POWER_STATES][NUM_POWER_STATES] = {
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
  TRANSITION(POWER_FSM_STATE_AUX, POWER_FSM_SEND_PD_BMS),
  TRANSITION(POWER_FSM_CONFIRM_AUX_STATUS, POWER_FSM_SEND_PD_BMS),
  TRANSITION(POWER_FSM_SEND_PD_BMS, POWER_FSM_CONFIRM_BATTERY_STATUS),
  TRANSITION(POWER_FSM_CONFIRM_BATTERY_STATUS, POWER_FSM_CLOSE_BATTERY_RELAYS),
  TRANSITION(POWER_FSM_CLOSE_BATTERY_RELAYS, POWER_FSM_CONFIRM_DC_DC),
  TRANSITION(POWER_FSM_CONFIRM_DC_DC, POWER_FSM_TURN_ON_EVERYTHING),
  TRANSITION(POWER_FSM_TURN_ON_EVERYTHING, POWER_FSM_POWER_MAIN_COMPLETE),
  TRANSITION(POWER_FSM_POWER_MAIN_COMPLETE, POWER_FSM_STATE_MAIN),

  // Sequence into AUX state
  TRANSITION(POWER_FSM_CONFIRM_AUX_STATUS, POWER_FSM_TURN_ON_EVERYTHING),
  TRANSITION(POWER_FSM_TURN_ON_EVERYTHING, POWER_FSM_STATE_AUX),

  // Failures when attempting OFF -> MAIN state
  // Failures when attempting OFF -> AUX
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
  TRANSITION(POWER_FSM_CLOSE_BATTERY_RELAYS, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_CONFIRM_DC_DC, POWER_FSM_STATE_AUX),
  TRANSITION(POWER_FSM_POWER_MAIN_COMPLETE, POWER_FSM_STATE_AUX),
};

StatusCode init_power_fsm(PowerFsmStateId inital_state) {
  // Assuming GPIOs have already been initialized in main
  // fsm_shared_mem_init();
  power_context.latest_state = 0;
  power_context.target_state = 0;
  fsm_init(power, s_power_fsm_states, s_power_transitions, inital_state, NULL);

  // Start button interrupt
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_RISING,  // Todo (Bafran): Double check if this is normally open
  };
  gpio_it_register_interrupt(&s_btn_start, &it_settings, START_BUTTON_EVENT, power);
  return STATUS_CODE_OK;
}
