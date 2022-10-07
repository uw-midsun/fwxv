#include "power_select_power_supply_task.h"

PowerSupplyStorage s_storage;

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
  // should we be setting over* to false
  s_storage.valid = false;
  s_storage.overvoltage = false;
  s_storage.overcurrent = false;
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
  adc_read_converted(g_power_select_voltage_pin, &s_storage.voltage);
  if (s_storage.voltage > POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV) {
    LOG_WARN("power_supply: overvoltage");
    s_storage.overvoltage = true;
  }
  adc_read_converted(g_power_select_current_pin, &s_storage.current);
  if (s_storage.current > POWER_SELECT_PWR_SUP_MAX_CURRENT_MA) {
    LOG_WARN("power_supply: overcurrent");
    s_storage.overcurrent = true;
  }
  // logging format?
  LOG_DEBUG("power_supply: valid=%d, voltage=%d, current=%d", state == GPIO_STATE_HIGH,
            s_storage.voltage, s_storage.current);
  s_storage.valid = true;
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