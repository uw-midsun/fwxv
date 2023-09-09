#include "power_select.h"
#include "power_select_setters.h"

#define AUX_BAT_STATUS g_tx_struct.power_select_status_status
#define AUX_BAT_FAULT g_tx_struct.power_select_status_fault

const GpioAddress g_aux_bat_valid_pin = POWER_SELECT_AUX_BAT_VALID_ADDR;
const GpioAddress g_aux_bat_voltage_pin = POWER_SELECT_AUX_BAT_VSENSE_ADDR;
const GpioAddress g_aux_bat_current_pin = POWER_SELECT_AUX_BAT_ISENSE_ADDR;
const GpioAddress g_aux_bat_temp_pin = POWER_SELECT_AUX_BAT_TSENSE_ADDR;

static uint16_t adc_reading_voltage;
static uint16_t adc_reading_current;
static uint16_t adc_reading_temp;

FSM(aux_bat, NUM_POWER_SELECT_STATES);

static void prv_aux_bat_inactive_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_aux_bat_valid_pin, &state);
  if (state == GPIO_STATE_LOW) {
    fsm_transition(fsm, POWER_SELECT_ACTIVE);
  }
  LOG_DEBUG("aux_bat: valid=%d\n", state == GPIO_STATE_HIGH);
}

static void prv_aux_bat_inactive_output(void *context) {
  set_power_select_status_status(AUX_BAT_STATUS & ~POWER_SELECT_AUX_BAT_STATUS_MASK);
  LOG_DEBUG("aux_bat: transitioned to INACTIVE\n");
}

// Logic when power supply status is active (monitoring inputs)
static void prv_aux_bat_active_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_aux_bat_valid_pin, &state);
  if (state == GPIO_STATE_HIGH) {
    fsm_transition(fsm, POWER_SELECT_INACTIVE);
    return;
  }
  set_power_select_status_fault(AUX_BAT_FAULT & ~(POWER_SELECT_AUX_BAT_FAULT_OC_MASK |
                                                  POWER_SELECT_AUX_BAT_FAULT_OV_MASK |
                                                  POWER_SELECT_AUX_BAT_FAULT_OT_MASK));
  adc_read_converted(g_aux_bat_voltage_pin, &adc_reading_voltage);
  set_power_select_dcdc_measurements_dcdc_voltage(adc_reading_voltage);
  if (adc_reading_voltage > POWER_SELECT_AUX_BAT_MAX_VOLTAGE_MV) {
    LOG_WARN("aux_bat: overvoltage\n");
    set_power_select_status_fault(AUX_BAT_FAULT | POWER_SELECT_AUX_BAT_FAULT_OV_MASK);
  }
  adc_read_converted(g_aux_bat_current_pin, &adc_reading_current);
  set_power_select_dcdc_measurements_dcdc_current(adc_reading_current);
  if (adc_reading_current > POWER_SELECT_AUX_BAT_MAX_CURRENT_MA) {
    LOG_WARN("aux_bat: overcurrent\n");
    set_power_select_status_fault(AUX_BAT_FAULT | POWER_SELECT_AUX_BAT_FAULT_OC_MASK);
  }
  adc_read_converted(g_aux_bat_temp_pin, &adc_reading_temp);
  set_power_select_dcdc_measurements_dcdc_temp(adc_reading_temp);
  if (adc_reading_temp > POWER_SELECT_AUX_BAT_MAX_TEMP_C) {
    LOG_WARN("aux_bat: overtemperature\n");
    set_power_select_status_fault(AUX_BAT_FAULT | POWER_SELECT_AUX_BAT_FAULT_OT_MASK);
  }
  LOG_DEBUG("aux_bat: valid=%d, voltage=%d, current=%d, temp=%d\n", state == GPIO_STATE_HIGH,
            adc_reading_voltage, adc_reading_current, adc_reading_temp);
}

// Set the power supply status to active
static void prv_aux_bat_active_output(void *context) {
  set_power_select_status_status(AUX_BAT_STATUS | POWER_SELECT_AUX_BAT_STATUS_MASK);
  LOG_DEBUG("aux_bat: transitioned to ACTIVE\n");
}

// Define FSM State and Transition
static FsmState s_aux_bat_state_list[NUM_POWER_SELECT_STATES] = {
  STATE(POWER_SELECT_INACTIVE, prv_aux_bat_inactive_input, prv_aux_bat_inactive_output),
  STATE(POWER_SELECT_ACTIVE, prv_aux_bat_active_input, prv_aux_bat_active_output),
};

static FsmTransition s_aux_bat_transition_list[NUM_POWER_SELECT_STATES] = {
  TRANSITION(POWER_SELECT_INACTIVE, POWER_SELECT_ACTIVE),
  TRANSITION(POWER_SELECT_ACTIVE, POWER_SELECT_INACTIVE),
};

StatusCode init_aux_bat(void) {
  // Initialize valid GPIO pin
  status_ok_or_return(gpio_init_pin(&g_aux_bat_valid_pin, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));

  // Initialize voltage & current pins, and set adc channels
  status_ok_or_return(gpio_init_pin(&g_aux_bat_voltage_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_aux_bat_voltage_pin));
  status_ok_or_return(gpio_init_pin(&g_aux_bat_current_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_aux_bat_current_pin));
  status_ok_or_return(gpio_init_pin(&g_aux_bat_temp_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_aux_bat_temp_pin));

  // Initialize FSM task
  const FsmSettings settings = {
    .state_list = s_aux_bat_state_list,
    .transitions = s_aux_bat_transition_list,
    .num_transitions = NUM_POWER_SELECT_TRANSITIONS,
    .initial_state = POWER_SELECT_INACTIVE,
  };
  fsm_init(aux_bat, settings, NULL);

  return STATUS_CODE_OK;
}
