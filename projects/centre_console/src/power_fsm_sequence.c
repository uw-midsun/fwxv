#include "power_fsm_sequence.h"

// Input/outputs for going into MAIN

void prv_power_fsm_confirm_aux_status_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_send_pd_bms_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_confirm_battery_status_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_close_battery_relays_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_confirm_dc_dc_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_power_main_complete_input(Fsm *fsm, void *context) {
  return;
}

void prv_power_fsm_confirm_aux_status_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to confirm aux status\n");
}

void prv_power_fsm_send_pd_bms_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to send pd bms\n");
}

void prv_power_fsm_confirm_battery_status_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to confirm battery status\n");
}

void prv_power_fsm_close_battery_relays_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to close battery relays\n");
}

void prv_power_fsm_confirm_dc_dc_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to confirm dc dc\n");
}

void prv_power_fsm_power_main_complete_output(Fsm *fsm, void *context) {
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

void prv_power_fsm_discharge_precharge_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to discharge precharge\n");
}

void prv_power_fsm_turn_off_everything_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to turn off everything\n");
}

void prv_power_fsm_open_relays_output(Fsm *fsm, void *context) {
  LOG_DEBUG("Transitioned to open relays\n");
}
