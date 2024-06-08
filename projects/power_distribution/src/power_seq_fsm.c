#include "power_seq_fsm.h"

#include "outputs.h"
#include "pd_fault.h"
#include "persist.h"
#include "power_distribution_getters.h"
#include "power_distribution_setters.h"

static PersistStorage s_persist;
static BpsStorage s_bps_storage;

#define pd_fault_ok_or_transition(fsm)                                     \
  power_context.fault = check_pd_fault();                                  \
  if (check_battery_status_msg_watchdog() || get_battery_status_fault()) { \
    fsm_transition(fsm, POWER_STATE_FAULT);                                \
    return;                                                                \
  }

FSM(power_seq, NUM_POWER_STATES, TASK_STACK_256);

static PowerFsmContext power_context = { 0 };

static void prv_off_state_output(void *context) {
  LOG_DEBUG("Transitioned to OFF STATE\n");
  pd_set_active_output_group(OUTPUT_GROUP_POWER_OFF);
  power_context.latest_state = POWER_STATE_OFF;
  set_pd_status_power_state(EE_POWER_OFF_STATE);
}

static void prv_off_state_input(Fsm *fsm, void *context) {
  // On initialization, DCDC is not on, so only check aux
  if (check_battery_status_msg_watchdog() || check_aux_fault()) {
    fsm_transition(fsm, POWER_STATE_FAULT);
    return;
  }
  LOG_DEBUG("IN off state - %d\n", get_received_cc_info());
  LOG_DEBUG("s_bps_storage.fault_bitset %d\n", s_bps_storage.fault_bitset);
  if (!get_received_cc_info()) {
    return;
  }
  CentreConsoleDriveState cc_drive_event = get_cc_info_drive_state();
  if (cc_drive_event == EE_DRIVE_OUTPUT_DRIVE_STATE ||
      cc_drive_event == EE_DRIVE_OUTPUT_REVERSE_STATE) {
    power_context.target_state = POWER_STATE_PRECHARGE;
    fsm_transition(fsm, POWER_STATE_PRECHARGE);
  }
}

static void prv_precharge_output(void *context) {
  LOG_DEBUG("Transitioned to PRECHARGE STATE\n");
  pd_set_active_output_group(OUTPUT_GROUP_POWER_DRIVE);
  set_pd_status_power_state(EE_POWER_PRECHARGE_STATE);
  power_context.latest_state = POWER_STATE_OFF;
  power_context.timer_start_ticks = xTaskGetTickCount();
}

static void prv_precharge_input(Fsm *fsm, void *context) {
  LOG_DEBUG("GET_MOTOR_VELOCITY %d\n",
            (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()));
  if (get_motor_velocity_velocity_l() && get_motor_velocity_velocity_r()) {
    fsm_transition(fsm, POWER_STATE_OFF);
  }
  if (get_mc_status_precharge_status()) {
    fsm_transition(fsm, POWER_STATE_DRIVE);
  } else if ((xTaskGetTickCount() - power_context.timer_start_ticks) >
             pdMS_TO_TICKS(MCI_RESPONSE_TIMEOUT_MS)) {
    fsm_transition(fsm, POWER_STATE_OFF);
  }
}

static void prv_drive_state_output(void *context) {
  LOG_DEBUG("Transitioned to DRIVE STATE\n");
  pd_set_active_output_group(OUTPUT_GROUP_POWER_DRIVE);
  power_context.latest_state = POWER_STATE_DRIVE;
  s_bps_storage.fault_bitset = 0;
  s_bps_storage.vehicle_speed =
      (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) / 2;
  persist_commit(&s_persist);
  set_pd_status_bps_persist(false);
  set_pd_status_power_state(EE_POWER_DRIVE_STATE);
}

static void prv_drive_state_input(Fsm *fsm, void *context) {
  pd_fault_ok_or_transition(fsm);
  if (!get_received_cc_info()) {
    return;
  }
  CentreConsoleDriveState cc_drive_event = get_cc_info_drive_state();
  if (cc_drive_event == EE_DRIVE_OUTPUT_NEUTRAL_STATE) {
    power_context.target_state = POWER_STATE_OFF;
    fsm_transition(fsm, POWER_STATE_OFF);
  }
}

static void prv_fault_state_output(void *context) {
  LOG_DEBUG("Transitioned to FAULT STATE\n");
  s_bps_storage.fault_bitset = get_battery_status_fault();
  s_bps_storage.vehicle_speed =
      (get_motor_velocity_velocity_l() + get_motor_velocity_velocity_r()) / 2;
  persist_commit(&s_persist);
  pd_set_active_output_group(OUTPUT_GROUP_POWER_FAULT);
  set_pd_status_bps_persist(s_bps_storage.fault_bitset);
  set_pd_status_power_state(EE_POWER_FAULT_STATE);
  // TODO(devAdhiraj): start bps strobe
}

static void prv_fault_state_input(Fsm *fsm, void *context) {}

// Power Sequence FSM declaration for states and transitions
static FsmState s_power_seq_state_list[NUM_POWER_STATES] = {
  STATE(POWER_STATE_OFF, prv_off_state_input, prv_off_state_output),
  STATE(POWER_STATE_PRECHARGE, prv_precharge_input, prv_precharge_output),
  STATE(POWER_STATE_DRIVE, prv_drive_state_input, prv_drive_state_output),
  STATE(POWER_STATE_FAULT, prv_fault_state_input, prv_fault_state_output),
};

static bool s_power_seq_transitions[NUM_POWER_STATES][NUM_POWER_STATES] = {
  TRANSITION(POWER_STATE_OFF, POWER_STATE_FAULT),
  TRANSITION(POWER_STATE_OFF, POWER_STATE_PRECHARGE),
  TRANSITION(POWER_STATE_PRECHARGE, POWER_STATE_OFF),
  TRANSITION(POWER_STATE_PRECHARGE, POWER_STATE_DRIVE),
  TRANSITION(POWER_STATE_PRECHARGE, POWER_STATE_FAULT),
  TRANSITION(POWER_STATE_DRIVE, POWER_STATE_OFF),
  TRANSITION(POWER_STATE_DRIVE, POWER_STATE_FAULT)
};

StatusCode init_power_seq(void) {
  memset(&s_bps_storage, 0, sizeof(s_bps_storage));
  persist_init(&s_persist, BPS_FAULT_FLASH_PAGE, &s_bps_storage, sizeof(s_bps_storage), true);
  persist_ctrl_periodic(&s_persist, false);
  if (s_bps_storage.fault_bitset) set_pd_status_bps_persist(s_bps_storage.fault_bitset);

  fsm_init(power_seq, s_power_seq_state_list, s_power_seq_transitions, POWER_STATE_OFF, NULL);
  return STATUS_CODE_OK;
}
