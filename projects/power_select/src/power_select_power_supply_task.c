#include "power_select_power_supply_task.h"

#define PWR_SUP_STATUS g_tx_struct.power_select_status_status
#define PWR_SUP_FAULT g_tx_struct.power_select_status_fault
#define PWR_SUP_V g_tx_struct.power_select_dcdc_measurements_power_supply_voltage
#define PWR_SUP_C g_tx_struct.power_select_aux_measurements_power_supply_current

const GpioAddress g_power_select_valid_pin = POWER_SELECT_PWR_SUP_VALID_ADDR;
const GpioAddress g_power_select_voltage_pin = POWER_SELECT_PWR_SUP_VSENSE_ADDR;
const GpioAddress g_power_select_current_pin = POWER_SELECT_PWR_SUP_ISENSE_ADDR;

FSM(power_supply, NUM_POWER_SUPPLY_STATES);

static void prv_power_supply_inactive_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_power_select_valid_pin, &state);
  if (state == GPIO_STATE_LOW) {
    fsm_transition(fsm, POWER_SUPPLY_ACTIVE);
  }
  // should we be logging valid pin value or if its valid?
  LOG_DEBUG("power_supply: valid=%d", state == GPIO_STATE_HIGH);
  // set power supply bits to 0
  PWR_SUP_STATUS &= ~POWER_SELECT_PWR_SUP_STATUS_MASK;
  PWR_SUP_FAULT &= ~(POWER_SELECT_PWR_SUP_FAULT_OC_MASK | POWER_SELECT_PWR_SUP_FAULT_OV_MASK);
}

static void prv_power_supply_inactive_output(void *context) {
  // is this logging necessary
  LOG_DEBUG("Transitioned to POWER_SUPPLY_INACTIVE\n");
}

static void prv_power_supply_active_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_power_select_valid_pin, &state);
  if (state == GPIO_STATE_HIGH) {
    fsm_transition(fsm, POWER_SUPPLY_INACTIVE);
    return;
  }
  adc_read_converted(g_power_select_voltage_pin, &PWR_SUP_V);
  if (PWR_SUP_V > POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV) {
    LOG_WARN("power_supply: overvoltage");
    PWR_SUP_FAULT |= POWER_SELECT_PWR_SUP_FAULT_OV_MASK;
  }
  adc_read_converted(g_power_select_current_pin, &PWR_SUP_C);
  if (PWR_SUP_C > POWER_SELECT_PWR_SUP_MAX_CURRENT_MA) {
    LOG_WARN("power_supply: overcurrent");
    PWR_SUP_FAULT |= POWER_SELECT_PWR_SUP_FAULT_OC_MASK;
  }
  // logging format?
  LOG_DEBUG("power_supply: valid=%d, voltage=%d, current=%d", state == GPIO_STATE_HIGH, PWR_SUP_V,
            PWR_SUP_C);
  PWR_SUP_STATUS |= POWER_SELECT_PWR_SUP_STATUS_MASK;
}

static void prv_power_supply_active_output(void *context) {
  LOG_DEBUG("Transitioned to POWER_SUPPLY_ACTIVE\n");
}

static FsmState s_power_supply_state_list[NUM_POWER_SUPPLY_STATES] = {
  STATE(POWER_SUPPLY_INACTIVE, prv_power_supply_inactive_input, prv_power_supply_inactive_output),
  STATE(POWER_SUPPLY_ACTIVE, prv_power_supply_active_input, prv_power_supply_active_output),
};

static FsmTransition s_power_supply_transition_list[NUM_POWER_SUPPLY_STATES] = {
  TRANSITION(POWER_SUPPLY_INACTIVE, POWER_SUPPLY_ACTIVE),
  TRANSITION(POWER_SUPPLY_ACTIVE, POWER_SUPPLY_INACTIVE),
};

StatusCode init_power_supply(void) {
  // init valid pin
  const GpioSettings valid_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  status_ok_or_return(gpio_init_pin(&g_power_select_valid_pin, &valid_settings));
  // init voltage and current pins and set adc channel
  const GpioSettings sense_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  status_ok_or_return(gpio_init_pin(&g_power_select_voltage_pin, &sense_settings));
  status_ok_or_return(adc_set_channel(g_power_select_voltage_pin, true));
  status_ok_or_return(gpio_init_pin(&g_power_select_current_pin, &sense_settings));
  status_ok_or_return(adc_set_channel(g_power_select_current_pin, true));
  // init FSM task
  const FsmSettings settings = {
    .state_list = s_power_supply_state_list,
    .transitions = s_power_supply_transition_list,
    .num_transitions = NUM_POWER_SUPPLY_TRANSITIONS,
    .initial_state = POWER_SUPPLY_INACTIVE,
  };
  fsm_init(power_supply, settings, NULL);
  return STATUS_CODE_OK;
}