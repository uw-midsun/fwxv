#ifndef PROJECTS_CENTRE_CONSOLE_INC_POWER_FSM_SEQUENCE_H_
#define PROJECTS_CENTRE_CONSOLE_INC_POWER_FSM_SEQUENCE_H_

#include "fsm.h"

// Tell power select to check any aux faults before enabling power
void prv_power_fsm_confirm_aux_status_input(Fsm *fsm, void *context);
void prv_power_fsm_confirm_aux_status_output(void *context);

// Tell Power distribution to power on BMS board
void prv_power_fsm_send_pd_bms_input(Fsm *fsm, void *context);
void prv_power_fsm_send_pd_bms_output(void *context);

// Confirms battery checks, waits for ack
void prv_power_fsm_confirm_battery_status_input(Fsm *fsm, void *context);
void prv_power_fsm_confirm_battery_status_output(void *context);

// Transmits to BMS to close relays
void prv_power_fsm_close_battery_relays_input(Fsm *fsm, void *context);
void prv_power_fsm_close_battery_relays_output(void *context);

// Power Select confirms DCDC
void prv_power_fsm_confirm_dc_dc_input(Fsm *fsm, void *context);
void prv_power_fsm_confirm_dc_dc_output(void *context);

// Power Distribution enables all boards
void prv_power_fsm_turn_on_everything_input(Fsm *fsm, void *context);
void prv_power_fsm_turn_on_everything_output(void *context);

// Sends “ready to drive” to MCI
void prv_power_fsm_power_main_complete_input(Fsm *fsm, void *context);
void prv_power_fsm_power_main_complete_output(void *context);

// Send message to MCI to discharge precharge
void prv_power_fsm_discharge_precharge_input(Fsm *fsm, void *context);
void prv_power_fsm_discharge_precharge_output(void *context);

// Tell Power distribution to turn off the relevant boards
void prv_power_fsm_turn_off_everything_input(Fsm *fsm, void *context);
void prv_power_fsm_turn_off_everything_output(void *context);

// Tell BMS to open the relays
void prv_power_fsm_open_relays_input(Fsm *fsm, void *context);
void prv_power_fsm_open_relays_output(void *context);

#endif  // PROJECTS_CENTRE_CONSOLE_INC_POWER_FSM_SEQUENCE_H_
