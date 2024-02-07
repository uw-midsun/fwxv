#include "power_select.h"
#include "power_select_setters.h"

#define DCDC_STATUS g_tx_struct.power_select_status_status
#define DCDC_FAULT g_tx_struct.power_select_status_fault

const GpioAddress g_dcdc_valid_pin = POWER_SELECT_DCDC_VALID_ADDR;
const GpioAddress g_dcdc_voltage_pin = POWER_SELECT_DCDC_VSENSE_ADDR;
const GpioAddress g_dcdc_current_pin = POWER_SELECT_DCDC_ISENSE_ADDR;
const GpioAddress g_dcdc_temp_pin = POWER_SELECT_DCDC_TSENSE_ADDR;

static uint16_t adc_reading_voltage;
static uint16_t adc_reading_current;
static uint16_t adc_reading_temp;

FSM(dcdc, NUM_POWER_SELECT_STATES, TASK_STACK_512);

static void prv_dcdc_inactive_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_dcdc_valid_pin, &state);
  if (state == GPIO_STATE_LOW) {
    fsm_transition(fsm, POWER_SELECT_ACTIVE);
  }
  LOG_DEBUG("dcdc: valid=%d\n", state == GPIO_STATE_HIGH);
}

static void prv_dcdc_inactive_output(void *context) {
  set_power_select_status_status(DCDC_STATUS & ~POWER_SELECT_DCDC_STATUS_MASK);
  LOG_DEBUG("dcdc: transitioned to INACTIVE\n");
}

// Logic when power supply status is active (monitoring inputs)
static void prv_dcdc_active_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_dcdc_valid_pin, &state);
  if (state == GPIO_STATE_HIGH) {
    fsm_transition(fsm, POWER_SELECT_INACTIVE);
    return;
  }
  set_power_select_status_fault(DCDC_FAULT & ~(POWER_SELECT_DCDC_FAULT_OC_MASK |
                                               POWER_SELECT_DCDC_FAULT_OV_MASK |
                                               POWER_SELECT_DCDC_FAULT_OT_MASK));
  adc_read_converted(g_dcdc_voltage_pin, &adc_reading_voltage);
  set_power_select_dcdc_measurements_dcdc_voltage(adc_reading_voltage);
  if (adc_reading_voltage > POWER_SELECT_DCDC_MAX_VOLTAGE_MV) {
    LOG_WARN("dcdc: overvoltage\n");
    set_power_select_status_fault(DCDC_FAULT | POWER_SELECT_DCDC_FAULT_OV_MASK);
  }
  adc_read_converted(g_dcdc_current_pin, &adc_reading_current);
  set_power_select_dcdc_measurements_dcdc_current(adc_reading_current);
  if (adc_reading_current > POWER_SELECT_DCDC_MAX_CURRENT_MA) {
    LOG_WARN("dcdc: overcurrent\n");
    set_power_select_status_fault(DCDC_FAULT | POWER_SELECT_DCDC_FAULT_OC_MASK);
  }
  adc_read_converted(g_dcdc_temp_pin, &adc_reading_temp);
  set_power_select_dcdc_measurements_dcdc_temp(adc_reading_temp);
  if (adc_reading_temp > POWER_SELECT_DCDC_MAX_TEMP_C) {
    LOG_WARN("dcdc: overtemperature\n");
    set_power_select_status_fault(DCDC_FAULT | POWER_SELECT_DCDC_FAULT_OT_MASK);
  }
  LOG_DEBUG("dcdc: valid=%d, voltage=%d, current=%d, temp=%d\n", state == GPIO_STATE_HIGH,
            adc_reading_voltage, adc_reading_current, adc_reading_temp);
}

// Set the power supply status to active
static void prv_dcdc_active_output(void *context) {
  set_power_select_status_status(DCDC_STATUS | POWER_SELECT_DCDC_STATUS_MASK);
  LOG_DEBUG("dcdc: transitioned to ACTIVE\n");
}

// Define FSM State and Transition
static FsmState s_power_supply_state_list[NUM_POWER_SELECT_STATES] = {
  STATE(POWER_SELECT_INACTIVE, prv_dcdc_inactive_input, prv_dcdc_inactive_output),
  STATE(POWER_SELECT_ACTIVE, prv_dcdc_active_input, prv_dcdc_active_output),
};

static bool s_power_supply_transitions[NUM_POWER_SELECT_STATES][NUM_POWER_SELECT_STATES] = {
  TRANSITION(POWER_SELECT_INACTIVE, POWER_SELECT_ACTIVE),
  TRANSITION(POWER_SELECT_ACTIVE, POWER_SELECT_INACTIVE),
};

StatusCode init_dcdc(void) {
  // Initialize valid GPIO pin
  status_ok_or_return(gpio_init_pin(&g_dcdc_valid_pin, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW));

  // Initialize voltage & current pins, and set adc channels
  status_ok_or_return(gpio_init_pin(&g_dcdc_voltage_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_dcdc_voltage_pin));
  status_ok_or_return(gpio_init_pin(&g_dcdc_current_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_dcdc_current_pin));
  status_ok_or_return(gpio_init_pin(&g_dcdc_temp_pin, GPIO_ANALOG, GPIO_STATE_LOW));
  status_ok_or_return(adc_add_channel(g_dcdc_temp_pin));

  // Initialize FSM task
  fsm_init(dcdc, s_power_supply_state_list, s_power_supply_transitions, POWER_SELECT_INACTIVE,
           NULL);

  return STATUS_CODE_OK;
}
