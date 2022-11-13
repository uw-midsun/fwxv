#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "adc.h"
#include "delay.h"      // For real-time delays
#include "gpio.h"       // General purpose I/O control.
#include "interrupt.h"  // For enabling interrupts
#include "log.h"        // For outputting to the terminal
#include "tasks.h"      // For tasks

/*
Finally the function should also initialize the FSM task (dcdc_task).
The dcdc_task will have 2 states: ACTIVE, INACTIVE

In ACTIVE state:
  1. Reads valid GPIO for valid measurements
       Moves to INACTIVE if not valid
  2. Reads the voltage
      If overvoltage sets the FAULT_STATUS to HIGH. See CAN Message below for details
      Sets DCDC_VOLTAGE in CAN message
  3. Reads the current
      If overcurrent sets the FAULT_STATUS to HIGH. See CAN Message below for details
      Sets DCDC_CURRENT in CAN message
  4. Reads the temperature
      If overtemperature sets the FAULT_STATUS to HIGH. See CAN Message below for details
      Sets DCDC_TEMP in CAN message
  5. Sets STATUS to HIGH

In INACTIVE state:
1. Reads valid GPIO
      Moves back to ACTIVE if valid
2. Sets FAULT_STATUS and STATUS to LOW if not set. See CAN Message below for details
*/

#include "power_select_dcdc_task.h"
// #include "power_select_setters.h"

#define PWR_SUP_STATUS g_tx_struct.power_select_status_status
#define PWR_SUP_FAULT g_tx_struct.power_select_status_fault

const GpioAddress g_power_select_valid_pin = POWER_SELECT_PWR_SUP_VALID_ADDR;
const GpioAddress g_power_select_voltage_pin = POWER_SELECT_PWR_SUP_VSENSE_ADDR;
const GpioAddress g_power_select_current_pin = POWER_SELECT_PWR_SUP_ISENSE_ADDR;
const GpioAddress g_power_select_temp_pin = POWER_SELECT_PWR_SUP_TSENSE_ADDR;

static uint16_t adc_reading_voltage;
static uint16_t adc_reading_current;
static uint16_t adc_reading_temp;

// Logic when power supply status is inactive (monitoring inputs)
static void prv_power_supply_inactive_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_power_select_valid_pin, &state);
  if (state == GPIO_STATE_LOW) {
    fsm_transition(fsm, POWER_SUPPLY_ACTIVE);
  }
  LOG_DEBUG("power_supply: valid=%d", state == GPIO_STATE_HIGH);
}

// Set the power supply status to inactive
static void prv_power_supply_inactive_output(void *context) {
  set_power_select_status_status(PWR_SUP_STATUS & ~POWER_SELECT_DCDC_STATUS_MASK);
  LOG_DEBUG("Transitioned to POWER_SUPPLY_INACTIVE\n");
}

// Logic when power supply status is active (monitoring inputs)
static void prv_power_supply_active_input(Fsm *fsm, void *context) {
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&g_power_select_valid_pin, &state);
  if (state == GPIO_STATE_HIGH) {
    fsm_transition(fsm, POWER_SUPPLY_INACTIVE);
    return;
  }
  set_power_select_status_fault(
      PWR_SUP_FAULT & ~(POWER_SELECT_DCDC_FAULT_OC_MASK | POWER_SELECT_DCDC_FAULT_OV_MASK));
  adc_read_converted(g_power_select_voltage_pin, &adc_reading_voltage);
  set_power_select_dcdc_measurements_power_supply_voltage(adc_reading_voltage);
  if (adc_reading_voltage > POWER_SELECT_PWR_SUP_MAX_VOLTAGE_MV) {
    LOG_WARN("power_supply: overvoltage");
    set_power_select_status_fault(PWR_SUP_FAULT | POWER_SELECT_DCDC_FAULT_OV_MASK);
  }
  adc_read_converted(g_power_select_current_pin, &adc_reading_current);
  set_power_select_aux_measurements_power_supply_current(adc_reading_current);
  if (adc_reading_current > POWER_SELECT_PWR_SUP_MAX_CURRENT_MA) {
    LOG_WARN("power_supply: overcurrent");
    set_power_select_status_fault(PWR_SUP_FAULT | POWER_SELECT_DCDC_FAULT_OC_MASK);
  }
  adc_read_converted(g_power_select_temp_pin, &adc_reading_temp);
  set_power_select_dcdc_measurements_power_supply_temp(adc_reading_temp);
  if (adc_reading_temp > POWER_SELECT_PWR_SUP_MAX_TEMP_C) {
    LOG_WARN("power_supply: overtemperature");
    set_power_select_status_fault(PWR_SUP_FAULT | POWER_SELECT_DCDC_FAULT_OT_MASK);
  }
  LOG_DEBUG("power_supply: valid=%d, voltage=%d, current=%d, temp=%d", state == GPIO_STATE_HIGH,
            adc_reading_voltage, adc_reading_current, adc_reading_temp);
}

// Set the power supply status to active
static void prv_power_supply_active_output(void *context) {
  set_power_select_status_status(PWR_SUP_STATUS | POWER_SELECT_DCDC_STATUS_MASK);
  LOG_DEBUG("Transitioned to POWER_SUPPLY_ACTIVE\n");
}

FSM(dcdc, NUM_POWER_SUPPLY_STATES);

// Define FSM State and Transition
static FsmState s_power_supply_state_list[NUM_POWER_SUPPLY_STATES] = {
  STATE(POWER_SUPPLY_INACTIVE, prv_power_supply_inactive_input, prv_power_supply_inactive_output),
  STATE(POWER_SUPPLY_ACTIVE, prv_power_supply_active_input, prv_power_supply_active_output),
};

static FsmTransition s_power_supply_transition_list[NUM_POWER_SUPPLY_STATES] = {
  TRANSITION(POWER_SUPPLY_INACTIVE, POWER_SUPPLY_ACTIVE),
  TRANSITION(POWER_SUPPLY_ACTIVE, POWER_SUPPLY_INACTIVE),
};

StatusCode init_dcdc(void) {
  // Enable various peripherals
  log_init();
  interrupt_init();
  gpio_init();

  // Initialize valid GPIO pin
  static GpioSettings valid_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };
  status_ok_or_return(gpio_init_pin(&g_power_select_valid_pin, &valid_settings));

  // Initialize voltage & current pins, and set adc channels
  const GpioSettings sensor_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  status_ok_or_return(gpio_init_pin(&g_power_select_voltage_pin, &sensor_settings));
  status_ok_or_return(adc_set_channel(g_power_select_voltage_pin, true));
  status_ok_or_return(gpio_init_pin(&g_power_select_current_pin, &sensor_settings));
  status_ok_or_return(adc_set_channel(g_power_select_current_pin, true));
  status_ok_or_return(gpio_init_pin(&g_power_select_temp_pin, &sensor_settings));
  status_ok_or_return(adc_set_channel(g_power_select_temp_pin, true));

  // Initialize FSM task
  const FsmSettings settings = {
    .state_list = s_power_supply_state_list,
    .transitions = s_power_supply_transition_list,
    .num_transitions = NUM_POWER_SUPPLY_TRANSITIONS,
    .initial_state = POWER_SUPPLY_INACTIVE,
  };
  fsm_init(dcdc, settings, NULL);

  return STATUS_CODE_OK;
}
