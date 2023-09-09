#pragma once

#include "fsm.h"

// Tell power select to check any aux faults before enabling power
void power_fsm_confirm_aux_status_output(void *context);
void power_fsm_confirm_aux_status_input(Fsm *fsm, void *context);

// Tell Power distribution to power on BMS board
void power_fsm_send_pd_bms_output(void *context);
void power_fsm_send_pd_bms_input(Fsm *fsm, void *context);

// Confirms battery checks, waits for ack
void power_fsm_confirm_battery_status_output(void *context);
void power_fsm_confirm_battery_status_input(Fsm *fsm, void *context);

// Transmits to BMS to close relays
void power_fsm_close_battery_relays_output(void *context);
void power_fsm_close_battery_relays_input(Fsm *fsm, void *context);

// Power Select confirms DCDC
void power_fsm_confirm_dc_dc_output(void *context);
void power_fsm_confirm_dc_dc_input(Fsm *fsm, void *context);

// Power Distribution enables all boards
void power_fsm_turn_on_everything_output(void *context);
void power_fsm_turn_on_everything_input(Fsm *fsm, void *context);

// Sends “ready to drive” to MCI
void power_fsm_power_main_complete_output(void *context);
void power_fsm_power_main_complete_input(Fsm *fsm, void *context);

// Send message to MCI to discharge precharge
void power_fsm_discharge_precharge_output(void *context);
void power_fsm_discharge_precharge_input(Fsm *fsm, void *context);

// Tell Power distribution to turn off the relevant boards
void power_fsm_turn_off_everything_output(void *context);
void power_fsm_turn_off_everything_input(Fsm *fsm, void *context);

// Tell BMS to open the relays
void power_fsm_open_relays_output(void *context);
void power_fsm_open_relays_input(Fsm *fsm, void *context);
