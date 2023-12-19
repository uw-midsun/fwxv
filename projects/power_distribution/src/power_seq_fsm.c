#include "power_seq_fsm.h"

#include "outputs.h"
#include "power_distribution_getters.h"
#include "power_distribution_setters.h"

FSM(power_seq, NUM_POWER_STATES);

static PowerFsmContext power_context = { 0 };
static const GpioAddress dcdc_valid_pin = DCDC_VALID;

static GlobalErrorCode prv_fault_check() {
  if (check_battery_status_msg_watchdog()) {
    return PD_BMS_LOSS;
  }
  // TODO(devAdhiraj): Call PD fault checks function
  return NO_ERROR;
}

static void prv_off_state_output(void *context) {
  LOG_DEBUG("Transitioned to OFF STATE\n");
  power_context.latest_state = POWER_STATE_OFF;
  pd_set_active_output_group(OUTPUT_GROUP_POWER_OFF);
  set_power_info_power_state(EE_POWER_OFF_STATE);
}

static void prv_off_state_input(Fsm *fsm, void *context) {
  // TODO: Do we need to check fault here?
  if (!get_received_cc_power_control()) {
    return;
  }
  CentreConsoleCCPwrEvent cc_power_event = get_cc_power_control_power_event();
  if (cc_power_event == EE_CC_PWR_CTL_EVENT_BTN_AND_BRAKE) {
    power_context.target_state = POWER_STATE_DRIVE;
    fsm_transition(fsm, TRANSMIT_BMS_CLOSE_RELAYS);
  } else if (cc_power_event == EE_CC_PWR_CTL_EVENT_BTN) {
    power_context.target_state = POWER_STATE_ON;
    fsm_transition(fsm, TRANSMIT_BMS_CLOSE_RELAYS);
  }
}

static void prv_close_relays_state_output(void *context) {
  LOG_DEBUG("Transitioned to close relays STATE\n");
  pd_set_output_group(OUTPUT_GROUP_BMS_RELAYS, OUTPUT_STATE_ON);
  power_context.latest_state = TRANSMIT_BMS_CLOSE_RELAYS;
  power_context.timer_start_ticks = xTaskGetTickCount();
}

static void prv_close_relays_state_input(Fsm *fsm, void *context) {
  check_pd_fault(fsm);
  uint8_t bms_relay_state = get_battery_relay_info_state();
  if (bms_relay_state == EE_RELAY_STATE_CLOSE) {
    GpioState dcdc_state;
    if (gpio_get_state(&dcdc_valid_pin, &dcdc_state) == STATUS_CODE_OK &&
        dcdc_state == GPIO_STATE_LOW) {
      fsm_transition(fsm, POWER_STATE_OFF);
    } else {
      fsm_transition(fsm, POWER_STATE_ON);
    }
  } else if ((xTaskGetTickCount() - power_context.timer_start_ticks) >
             pdMS_TO_TICKS(BMS_RESPONSE_TIMEOUT_MS)) {
    fsm_transition(fsm, POWER_STATE_OFF);
  }
}

static void prv_on_state_output(void *context) {
  LOG_DEBUG("Transitioned to ON STATE\n");
  pd_set_active_output_group(OUTPUT_GROUP_POWER_ON);
  power_context.latest_state = POWER_STATE_ON;
  set_power_info_power_state(EE_POWER_ON_STATE);
}

static void prv_on_state_input(Fsm *fsm, void *context) {
  check_pd_fault(fsm);
  if (power_context.target_state == POWER_STATE_OFF) {
    fsm_transition(fsm, POWER_STATE_OFF);
    return;
  }
  if (power_context.target_state == POWER_STATE_DRIVE) {
    fsm_transition(fsm, TURN_ON_DRIVE_OUTPUTS);
    return;
  }

  if (!get_received_cc_power_control()) {
    return;
  }
  CentreConsoleCCPwrEvent cc_power_event = get_cc_power_control_power_event();
  if (cc_power_event == EE_CC_PWR_CTL_EVENT_BTN_AND_BRAKE) {
    power_context.target_state = POWER_STATE_DRIVE;
    fsm_transition(fsm, TURN_ON_DRIVE_OUTPUTS);
  } else if (cc_power_event == EE_CC_PWR_CTL_EVENT_BTN) {
    power_context.target_state = POWER_STATE_OFF;
    fsm_transition(fsm, POWER_STATE_OFF);
  }
}

static void prv_turn_on_drive_outputs_state_output(void *context) {
  LOG_DEBUG("Transitioned to TURN ON DRIVE STATE\n");
  pd_set_active_output_group(OUTPUT_GROUP_DRIVE);
  power_context.latest_state = TURN_ON_DRIVE_OUTPUTS;
  power_context.timer_start_ticks = xTaskGetTickCount();
}

static void prv_turn_on_drive_outputs_state_input(Fsm *fsm, void *context) {
  uint8_t mci_relay_state = get_precharge_completed_relay_status();
  if (mci_relay_state == EE_RELAY_STATE_CLOSE) {
    fsm_transition(fsm, POWER_STATE_DRIVE);
  } else if ((xTaskGetTickCount() - power_context.timer_start_ticks) >
             pdMS_TO_TICKS(MCI_RESPONSE_TIMEOUT_MS)) {
    fsm_transition(fsm, POWER_STATE_ON);
  }
}

static void prv_drive_state_output(void *context) {
  LOG_DEBUG("Transitioned to DRIVE STATE\n");
  power_context.latest_state = POWER_STATE_DRIVE;
  set_power_info_power_state(EE_POWER_DRIVE_STATE);
}

static void prv_drive_state_input(Fsm *fsm, void *context) {
  check_pd_fault(fsm);
  if (!get_received_cc_power_control()) {
    return;
  }
  CentreConsoleCCPwrEvent cc_power_event = get_cc_power_control_power_event();
  if (cc_power_event == EE_CC_PWR_CTL_EVENT_BTN_AND_BRAKE || EE_CC_PWR_CTL_EVENT_BTN) {
    power_context.target_state = POWER_STATE_ON;
    fsm_transition(fsm, POWER_STATE_ON);
  }
}

static void prv_fault_state_output(void *context) {
  LOG_DEBUG("Transitioned to FAULT STATE\n");
  if (power_context.latest_state == POWER_STATE_DRIVE ||
      power_context.latest_state == TURN_ON_DRIVE_OUTPUTS) {
    pd_set_output_group(OUTPUT_GROUP_DRIVE, OUTPUT_STATE_OFF);
  }
  pd_set_output_group(OUTPUT_GROUP_POWER_ON, OUTPUT_STATE_ON);
  set_power_info_pd_fault(power_context.fault);
  // TODO(devAdhiraj): start bps strobe
}

static void prv_fault_state_input(Fsm *fsm, void *context) {}

// Power Sequence FSM declaration for states and transitions
static FsmState s_power_seq_state_list[NUM_POWER_STATES] = {
  STATE(POWER_STATE_OFF, prv_off_state_input, prv_off_state_output),
  STATE(TRANSMIT_BMS_CLOSE_RELAYS, prv_close_relays_state_input, prv_close_relays_state_output),
  STATE(POWER_STATE_ON, prv_on_state_input, prv_on_state_output),
  STATE(TURN_ON_DRIVE_OUTPUTS, prv_turn_on_drive_outputs_state_input,
        prv_turn_on_drive_outputs_state_output),
  STATE(POWER_STATE_DRIVE, prv_drive_state_input, prv_drive_state_output),
  STATE(POWER_STATE_FAULT, prv_fault_state_input, prv_fault_state_output),
};

static bool s_power_seq_transitions[NUM_POWER_STATES][NUM_POWER_STATES] = {
  TRANSITION(POWER_STATE_OFF, TRANSMIT_BMS_CLOSE_RELAYS),
  TRANSITION(POWER_STATE_OFF, POWER_STATE_FAULT),
  TRANSITION(TRANSMIT_BMS_CLOSE_RELAYS, POWER_STATE_OFF),
  TRANSITION(TRANSMIT_BMS_CLOSE_RELAYS, POWER_STATE_ON),
  TRANSITION(TRANSMIT_BMS_CLOSE_RELAYS, POWER_STATE_FAULT),
  TRANSITION(POWER_STATE_ON, POWER_STATE_OFF),
  TRANSITION(POWER_STATE_ON, TURN_ON_DRIVE_OUTPUTS),
  TRANSITION(POWER_STATE_ON, POWER_STATE_FAULT),
  TRANSITION(TURN_ON_DRIVE_OUTPUTS, POWER_STATE_ON),
  TRANSITION(TURN_ON_DRIVE_OUTPUTS, POWER_STATE_DRIVE),
  TRANSITION(TURN_ON_DRIVE_OUTPUTS, POWER_STATE_FAULT),
  TRANSITION(POWER_STATE_DRIVE, POWER_STATE_ON),
  TRANSITION(POWER_STATE_DRIVE, POWER_STATE_FAULT),
};

StatusCode init_power_seq(void) {
  fsm_init(power_seq, s_power_seq_state_list, s_power_seq_transitions, POWER_STATE_OFF, NULL);
  gpio_init_pin(&dcdc_valid_pin, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}
