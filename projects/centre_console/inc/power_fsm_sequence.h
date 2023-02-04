#include "fsm.h"

void prv_power_fsm_confirm_aux_status_input(Fsm *fsm, void *context);
void prv_power_fsm_send_pd_bms_input(Fsm *fsm, void *context);
void prv_power_fsm_confirm_battery_status_input(Fsm *fsm, void *context);
void prv_power_fsm_close_battery_relays_input(Fsm *fsm, void *context);
void prv_power_fsm_confirm_dc_dc_input(Fsm *fsm, void *context);
void prv_power_fsm_power_main_complete_input(Fsm *fsm, void *context);
void prv_power_fsm_confirm_aux_status_output(Fsm *fsm, void *context);
void prv_power_fsm_send_pd_bms_output(Fsm *fsm, void *context);
void prv_power_fsm_confirm_battery_status_output(Fsm *fsm, void *context);
void prv_power_fsm_close_battery_relays_output(Fsm *fsm, void *context);
void prv_power_fsm_confirm_dc_dc_output(Fsm *fsm, void *context);
void prv_power_fsm_power_main_complete_output(Fsm *fsm, void *context);
void prv_power_fsm_discharge_precharge_input(Fsm *fsm, void *context);
void prv_power_fsm_turn_off_everything_input(Fsm *fsm, void *context);
void prv_power_fsm_open_relays_input(Fsm *fsm, void *context);
void prv_power_fsm_discharge_precharge_output(Fsm *fsm, void *context);
void prv_power_fsm_turn_off_everything_output(Fsm *fsm, void *context);
void prv_power_fsm_open_relays_output(Fsm *fsm, void *context);
