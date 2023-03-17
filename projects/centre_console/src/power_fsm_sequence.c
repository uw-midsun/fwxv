#include "power_fsm_sequence.h"
#include "power_fsm.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"

#include "log.h"

#define CONST 0

// Number of cyces to wait before falling back to stable state
static uint8_t cycle_timeout = 3;

// Input/outputs for going into MAIN

void prv_power_fsm_confirm_aux_status_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  uint8_t status = get_power_select_status_status();
  uint8_t fault = get_power_select_status_fault();

  // Status bit 2 is AUX, fault bits 5,6,7 are AUX
  if((status & 0x04) && !(fault & 0xE0)) {
    // Transition to next state
    if(state_context->target_state == POWER_FSM_STATE_MAIN) {
      fsm_transition(fsm, POWER_FSM_SEND_PD_BMS);
    } else {
      fsm_transition(fsm, POWER_FSM_STATE_AUX);
    }
  } else {
    // Transition to last stable state
    fsm_transition(fsm, state_context->latest_state);
  }
  return;
}

void prv_power_fsm_send_pd_bms_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  uint8_t rear_fault = get_rear_pd_fault_fault_data();
  uint8_t front_fault = get_front_pd_fault_fault_data();

  if(!(rear_fault) && !(front_fault)) {
    // Reset cycle counter
    cycle_timeout = 3;
    // Transition to next state
    fsm_transition(fsm, POWER_FSM_CONFIRM_BATTERY_STATUS);
  } else if (cycle_timeout == 0) {
    // Transition to last stable state
    fsm_transition(fsm, state_context->latest_state);
  } else {
    cycle_timeout--;
  }

  return;
}

void prv_power_fsm_confirm_battery_status_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  uint8_t status = get_bps_heartbeat_status();

  // TODO(Bafran): Confirm what bps hearbeat status message looks like
  if(!status) {
    // Reset cycle counter
    cycle_timeout = 3;
    // Transition to next state
    fsm_transition(fsm, POWER_FSM_CLOSE_BATTERY_RELAYS);
  } else if (cycle_timeout == 0) {
    // Transition to last stable state
    fsm_transition(fsm, state_context->latest_state);
  } else {
    cycle_timeout--;
  }

  return;
}

void prv_power_fsm_close_battery_relays_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  uint8_t hv_status = get_battery_relay_state_hv();
  uint8_t gnd_status = get_battery_relay_state_gnd();

  // If both relays are closed, transition to next sequence state
  if(hv_status && gnd_status) {
    // Transition to next state
    fsm_transition(fsm, POWER_FSM_CONFIRM_DC_DC);
  } else {
    // Transition to last stable state
    fsm_transition(fsm, state_context->latest_state);
  }
  return;
}

void prv_power_fsm_confirm_dc_dc_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;

  uint8_t status = get_power_select_status_status();
  uint8_t fault = get_power_select_status_fault();

  // Status bit 1 is AUX, fault bits 2, 3, 4 are AUX
  if((status & 0x02) && !(fault & 0x1C)) {
    // Transition to next state
    fsm_transition(fsm, POWER_FSM_TURN_ON_EVERYTHING);
  } else {
    // Transition to last stable state
    fsm_transition(fsm, state_context->latest_state);
  }

  return;
}

void prv_power_fsm_turn_on_everything_input(Fsm *fsm, void *context) {
  // No checks here, only "Turn on everything" message gets sent in the output function
  fsm_transition(fsm, POWER_FSM_POWER_MAIN_COMPLETE);
  return;
}

void prv_power_fsm_power_main_complete_input(Fsm *fsm, void *context) {
  // No checks here, only "Ready to drive" message gets sent in the output function
  fsm_transition(fsm, POWER_FSM_STATE_MAIN);
  return;
}

void prv_power_fsm_confirm_aux_status_output(void *context) {
  LOG_DEBUG("Transitioned to confirm aux status\n");
}

void prv_power_fsm_send_pd_bms_output(void *context) {
  set_set_bms_power_bms_power_on_notification(CONST);
  LOG_DEBUG("Transitioned to send pd bms\n");
}

void prv_power_fsm_confirm_battery_status_output(void *context) {
  LOG_DEBUG("Transitioned to confirm battery status\n");
}

void prv_power_fsm_close_battery_relays_output(void *context) {
  set_set_relay_states_relay_mask(CONST);
  set_set_relay_states_relay_state(CONST);
  LOG_DEBUG("Transitioned to close battery relays\n");
}

void prv_power_fsm_confirm_dc_dc_output(void *context) {
  LOG_DEBUG("Transitioned to confirm dc dc\n");
}

void prv_power_fsm_turn_on_everything_output(void *context) {
  set_set_power_state_turn_on_everything_notification(CONST);
  LOG_DEBUG("Transitioned to turn on everything\n");
}

void prv_power_fsm_power_main_complete_output(void *context) {
  LOG_DEBUG("Transitioned to power main complete\n");
}

// Input/outputs for going into AUX

// Confirm aux status
// Turn on everything

// Input/outputs for going into OFF

void prv_power_fsm_discharge_precharge_input(Fsm *fsm, void *context) {
  PowerFsmContext *state_context = (PowerFsmContext*)context;
  
  uint8_t precharge = get_precharge_completed_signal1();

  if(precharge) {
    // Transition to next state
    fsm_transition(fsm, POWER_FSM_TURN_OFF_EVERYTHING);
  } else {
    // Transition to last stable state
    fsm_transition(fsm, state_context->latest_state);
  }

  return;
}

void prv_power_fsm_turn_off_everything_input(Fsm *fsm, void *context) {
  // No checks here, only "Turn off everything" message gets sent in the output function
  fsm_transition(fsm, POWER_FSM_OPEN_RELAYS);
  return;
}

void prv_power_fsm_open_relays_input(Fsm *fsm, void *context) {

  PowerFsmContext *state_context = (PowerFsmContext*)context;

  uint8_t hv_status = get_battery_relay_state_hv();
  uint8_t gnd_status = get_battery_relay_state_gnd();

  // If both relays are open, transition to next sequence state
  if(hv_status && gnd_status) {
    // Transition to next state
    fsm_transition(fsm, POWER_FSM_DISCHARGE_PRECHARGE);
  } else {
    // Transition to last stable state
    fsm_transition(fsm, state_context->latest_state);
  }
  
  return;
}

void prv_power_fsm_discharge_precharge_output(void *context) {
  set_discharge_precharge_signal1(CONST);
  LOG_DEBUG("Transitioned to discharge precharge\n");
}

void prv_power_fsm_turn_off_everything_output(void *context) {
  // Maybe reuse this message somehow
  set_set_power_state_turn_on_everything_notification(!CONST);
  LOG_DEBUG("Transitioned to turn off everything\n");
}

void prv_power_fsm_open_relays_output(void *context) {
  set_set_relay_states_relay_mask(CONST);
  set_set_relay_states_relay_state(CONST);
  LOG_DEBUG("Transitioned to open relays\n");
}
