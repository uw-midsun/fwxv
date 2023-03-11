#include "power_fsm_sequence.h"
#include "power_fsm.h"
#include "centre_console_getters.h"
#include "centre_console_setters.h"

#include "log.h"

#define CONST 0

/** I'm assuming all statuses are 0 == "OK" and 1 == "SAD" because that's how status.h works
 *  This is pretty easy to change later just tedious
*/

// Input/outputs for going into MAIN

void prv_power_fsm_confirm_aux_status_input(Fsm *fsm, void *context) {
  uint8_t status = get_power_select_status_status();
  status = status | get_power_select_aux_measurements_power_supply_current(); // idk if we need this? is this covered in PS status?
  if(!status) {
    fsm_transition(fsm, POWER_FSM_SEND_PD_BMS);
  } else {
    fsm_transition(fsm, POWER_FSM_STATE_MAIN);
  }
  return;
}

void prv_power_fsm_send_pd_bms_input(Fsm *fsm, void *context) {
  uint8_t status = get_bps_heartbeat_status();
  // I guess we need to wait a couple cycles here, not sure how to do that
  return;
}

void prv_power_fsm_confirm_battery_status_input(Fsm *fsm, void *context) {
  uint8_t status = get_bps_heartbeat_status();
  if(!status) {
    fsm_transition(fsm, POWER_FSM_CLOSE_BATTERY_RELAYS);
  }
  return;
}

void prv_power_fsm_close_battery_relays_input(Fsm *fsm, void *context) {
  uint8_t hv_status = get_battery_relay_state_hv();
  uint8_t gnd_status = get_battery_relay_state_gnd();
  // If both relays are closed, transition to next sequence state
  if(hv_status && gnd_status) {
    fsm_transition(fsm, POWER_FSM_CONFIRM_DC_DC);
  }
  return;
}

void prv_power_fsm_confirm_dc_dc_input(Fsm *fsm, void *context) {
  uint16_t current = get_power_select_dcdc_measurements_dcdc_current();
  uint16_t temp = get_power_select_dcdc_measurements_dcdc_temp();
  uint16_t voltage = get_power_select_dcdc_measurements_dcdc_voltage();
  uint16_t ps_voltage = get_power_select_dcdc_measurements_power_supply_voltage();
  
  // I guess all of these need to be within some range?
  uint8_t status = 0;
  if(current < CONST || current > CONST) {
    status = 1;
  }
  if(temp < CONST || temp > CONST) {
    status = 2;
  }
  if(voltage < CONST || voltage > CONST) {
    status = 3;
  }
  if(ps_voltage < CONST || ps_voltage > CONST) {
    status = 4;
  }

  if(!status) {
    fsm_transition(fsm, POWER_FSM_TURN_ON_EVERYTHING);
  } else {
    fsm_transition(fsm, POWER_FSM_STATE_MAIN);
  }
  return;
}

void prv_power_fsm_turn_on_everything_input(Fsm *fsm, void *context) {
  // not sure where this ACK will come from, there's two pds i guess it'll need two acks?
  return;
}

void prv_power_fsm_power_main_complete_input(Fsm *fsm, void *context) {
  // check all the mci messages?
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
  return;
}

void prv_power_fsm_turn_off_everything_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_open_relays_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_discharge_precharge_output(void *context) {
  set_discharge_precharge_signal1(CONST);
  LOG_DEBUG("Transitioned to discharge precharge\n");
}

void prv_power_fsm_turn_off_everything_output(void *context) {
  // Maybe reuse this message somehow
  set_set_power_state_turn_on_everything_notification(CONST);
  LOG_DEBUG("Transitioned to turn off everything\n");
}

void prv_power_fsm_open_relays_output(void *context) {
  set_set_relay_states_relay_mask(CONST);
  set_set_relay_states_relay_state(CONST);
  LOG_DEBUG("Transitioned to open relays\n");
}
