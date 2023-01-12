#include "adc.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
// #include "new_can_tx_structs.h"
#include "power_select_setters.h"

#define AUX_BAT_STATUS g_tx_struct.power_select_status_status
#define AUX_BAT_FAULT g_tx_struct.power_select_status_fault
uint16_t threshold;

GpioAddress voltage_addr = {
  .port = GPIO_PORT_A,
  .pin = 1,
};
GpioAddress current_addr = {
  .port = GPIO_PORT_B,
  .pin = 1,
};
GpioAddress temp_addr = {
  .port = GPIO_PORT_C,
  .pin = 1,
};
GpioAddress valid_addr = {
  .port = GPIO_PORT_D,
  .pin = 1,
};
GpioAddress adc_current_addr = {
  .port = GPIO_PORT_E,
  .pin = 1,
};
GpioAddress adc_temp_addr = {
  .port = GPIO_PORT_F,
  .pin = 1,
};
GpioAddress adc_voltage_addr = {
  .port = GPIO_PORT_A,
  .pin = 2,
};

#define NUM_AUX_BAT_STATES 2
#define NUM_AUX_BAT_TRANSITIONS 2

DECLARE_FSM(aux_bat);

FSM(aux_bat, NUM_AUX_BAT_STATES);

typedef enum Aux_BatId {
  INACTIVE = 0,
  ACTIVE,
  NUM_STATES,
} Aux_BatId;

// Input Functions for the Two States
void prv_state0_input(Fsm *fsm, void *context) {
  GpioState valid_channel;
  gpio_get_state(&valid_addr, &valid_channel);
  if (valid_channel == GPIO_STATE_HIGH) {
    fsm_transition(fsm, ACTIVE);
  }
}

// Output Functions for the Two States
void prv_state0_output(void *context) {
  // Using AND to clear the aux_bat fault and status bits
  set_power_select_status_status(AUX_BAT_STATUS & ~(1 << 2));
  set_power_select_status_fault(AUX_BAT_FAULT & ~(1 << 7));
  set_power_select_status_fault(AUX_BAT_FAULT & ~(1 << 6));
  set_power_select_status_fault(AUX_BAT_FAULT & ~(1 << 5));
}

void prv_state1_input(Fsm *fsm, void *context) {
  GpioState valid_channel;
  gpio_get_state(&valid_addr, &valid_channel);
  if (valid_channel == GPIO_STATE_LOW) {
    fsm_transition(fsm, INACTIVE);
  }

  uint16_t adc_voltage_channel;
  adc_read_converted(adc_voltage_addr, &adc_voltage_channel);
  // This is a placeholder for different thresholds
  // uint16_t threshold;
  if (adc_voltage_channel > threshold) {
    // Set Fault Status to High
    // Set Aux Voltage
    set_power_select_status_fault(AUX_BAT_FAULT | (1 << 7));
    set_power_select_aux_measurements_aux_voltage(adc_voltage_channel);
  }
  uint16_t adc_current_channel;
  adc_read_converted(adc_current_addr, &adc_current_channel);
  if (adc_current_channel > threshold) {
    // Set Fault Status to High
    // Set Aux Current
    set_power_select_status_fault(AUX_BAT_FAULT | (1 << 6));
    set_power_select_aux_measurements_aux_current(adc_current_channel);
  }
  uint16_t adc_temp_channel;
  adc_read_converted(adc_temp_addr, &adc_temp_channel);
  if (adc_temp_channel > threshold) {
    // Set Fault Status to High
    // Set Aux Temp
    set_power_select_status_fault(AUX_BAT_FAULT | (1 << 5));
    set_power_select_aux_measurements_aux_temp(adc_temp_channel);
  }
}

void prv_state1_output(void *context) {
  // Using OR to set the aux_bat status bit
  set_power_select_status_status(AUX_BAT_STATUS | (1 << 2));
}

// Declare states in state list
static FsmState s_aux_bat_state_list[NUM_AUX_BAT_STATES] = {
  STATE(INACTIVE, prv_state0_input, prv_state0_output),
  STATE(ACTIVE, prv_state1_input, prv_state1_output),
};

// Declares transition for state machine, must match those in input functions
static FsmTransition s_aux_bat_transitions[NUM_AUX_BAT_TRANSITIONS] = {
  // Transitions for state 0
  TRANSITION(INACTIVE, ACTIVE),
  // Transitions for state 1
  TRANSITION(ACTIVE, INACTIVE),
};

StatusCode init_power_select_aux_bat() {
  status_ok_or_return(tasks_init_task(aux_bat, TASK_PRIORITY(2), NULL));
  return STATUS_CODE_OK;
}

StatusCode init_aux_bat(void) {
  // Initializations for gpio's, adc's, and the fsm
  gpio_init();
  gpio_init_pin(&voltage_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&current_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&temp_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&valid_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&adc_current_addr, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&adc_temp_addr, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&adc_voltage_addr, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(adc_current_addr);
  adc_add_channel(adc_temp_addr);
  adc_add_channel(adc_voltage_addr);
  adc_init(ADC_MODE_SINGLE);

  FsmSettings settings = {
    .state_list = s_aux_bat_state_list,
    .transitions = s_aux_bat_transitions,
    .num_transitions = NUM_AUX_BAT_TRANSITIONS,
    .initial_state = INACTIVE,
  };
  fsm_init(aux_bat, settings, NULL);
  return STATUS_CODE_OK;
}
