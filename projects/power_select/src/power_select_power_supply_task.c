#include "power_select.h"
#include "power_select_setters.h"

#define PWR_SUP_STATUS g_tx_struct.power_select_status_status
#define PWR_SUP_FAULT g_tx_struct.power_select_status_fault

const GpioAddress g_power_supply_valid_pin = POWER_SELECT_PWR_SUP_VALID_ADDR;
const GpioAddress g_power_supply_voltage_pin = POWER_SELECT_PWR_SUP_VSENSE_ADDR;
const GpioAddress g_power_supply_current_pin = POWER_SELECT_PWR_SUP_ISENSE_ADDR;

static uint16_t adc_reading_voltage;
static uint16_t adc_reading_current;

FSM(power_supply, NUM_POWER_SELECT_STATES);

static void prv_power_supply_inactive_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_power_supply_valid_pin, &state);
  if (state == GPIO_STATE_LOW) {
    fsm_transition(fsm, POWER_SELECT_ACTIVE);
  }
  LOG_DEBUG("power_supply: valid=%d\n", state == GPIO_STATE_HIGH);
}

static void prv_power_supply_inactive_output(void *context) {
  set_power_select_status_status(PWR_SUP_STATUS & ~POWER_SELECT_PWR_SUP_STATUS_MASK);
  LOG_DEBUG("power_supply: transitioned to INACTIVE\n\n");
}

static void prv_power_supply_active_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_power_supply_valid_pin, &state);
  if (state == GPIO_STATE_HIGH) {
    fsm_transition(fsm, POWER_SELECT_INACTIVE);
    return;
  }
  set_power_select_status_fault(
      PWR_SUP_FAULT & ~(POWER_SELECT_PWR_SUP_FAULT_OC_MASK | POWER_SELECT_PWR_SUP_FAULT_OV_MASK));
  adc_read_converted(g_power_supply_voltage_pin, &adc_reading_voltage);
  set_power_select_dcdc_measurements_power_supply_voltage(adc_reading_voltage);
  if (adc_reading_voltage > POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV) {
    LOG_WARN("power_supply: overvoltage\n");
    set_power_select_status_fault(PWR_SUP_FAULT | POWER_SELECT_PWR_SUP_FAULT_OV_MASK);
  }
  adc_read_converted(g_power_supply_current_pin, &adc_reading_current);
  set_power_select_aux_measurements_power_supply_current(adc_reading_current);
  if (adc_reading_current > POWER_SELECT_PWR_SUP_MAX_CURRENT_MA) {
    LOG_WARN("power_supply: overcurrent\n");
    set_power_select_status_fault(PWR_SUP_FAULT | POWER_SELECT_PWR_SUP_FAULT_OC_MASK);
  }
  LOG_DEBUG("power_supply: valid=%d, voltage=%d, current=%d\n", state == GPIO_STATE_HIGH,
            adc_reading_voltage, adc_reading_current);
}

static void prv_power_supply_active_output(void *context) {
  set_power_select_status_status(PWR_SUP_STATUS | POWER_SELECT_PWR_SUP_STATUS_MASK);
  LOG_DEBUG("power_supply: transitioned to ACTIVE\n");
}

static FsmState s_power_supply_state_list[NUM_POWER_SELECT_STATES] = {
  STATE(POWER_SELECT_INACTIVE, prv_power_supply_inactive_input, prv_power_supply_inactive_output),
  STATE(POWER_SELECT_ACTIVE, prv_power_supply_active_input, prv_power_supply_active_output),
};

static bool s_power_supply_transitions[NUM_POWER_SELECT_STATES][NUM_POWER_SELECT_STATES] = {
  TRANSITION(POWER_SELECT_INACTIVE, POWER_SELECT_ACTIVE),
  TRANSITION(POWER_SELECT_ACTIVE, POWER_SELECT_INACTIVE),
};

StatusCode init_power_supply(void) {
  // init valid pin
  status_ok_or_return(gpio_init_pin(&g_power_supply_valid_pin, GPIO_INPUT_PULL_UP, GPIO_STATE_LOW));
  status_ok_or_return(gpio_init_pin(&g_power_supply_voltage_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_power_supply_voltage_pin));
  status_ok_or_return(gpio_init_pin(&g_power_supply_current_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_power_supply_current_pin));
  // init FSM task
  const FsmSettings settings = {
    .state_list = s_power_supply_state_list,
    .transitions = *s_power_supply_transitions,
    .initial_state = POWER_SELECT_INACTIVE,
  };
  fsm_init(power_supply, settings, NULL);
  return STATUS_CODE_OK;
}
