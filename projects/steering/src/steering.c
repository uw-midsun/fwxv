#include "steering_task.h"

#define CC_INPUT g_tx_struct.steering_info_input_cc

static const GpioAddress turn_signal_address = TURN_SIGNAL_GPIO;
static const GpioAddress cc_address = CC_CHANGE_GPIO;
static const GpioAddress cc_toggle_address = CC_TOGGLE_GPIO;
static Event STEERING_EVENT;

StatusCode steering_init(Task *task) {
  // Initialize Pins
  gpio_init_pin(&turn_signal_address, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&cc_address, GPIO_ANALOG, GPIO_STATE_LOW);
  gpio_init_pin(&cc_toggle_address, GPIO_ANALOG, GPIO_STATE_LOW);
  // Set up ADC
  adc_add_channel(turn_signal_address);
  adc_add_channel(cc_address);
  // Initialize interrupt settings
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_FALLING,
  };
  // Initialize interrupt
  gpio_it_register_interrupt(&cc_toggle_address, &it_settings, CC_TOGGLE_EVENT, task);
  return STATUS_CODE_OK;
}

void steering_input(uint32_t notification) {
  uint16_t control_stalk_data;
  set_steering_info_input_lights(0);
  set_steering_info_input_cc(0);
  // Read ADC of pin set by turn signal lights
  adc_read_converted(turn_signal_address, &control_stalk_data);

  // Determine if it's a left, right or off signal
  if (control_stalk_data > TURN_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
      control_stalk_data < TURN_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    set_steering_info_input_lights(TURN_SIGNAL_LEFT);
  } else if (control_stalk_data > TURN_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
             control_stalk_data < TURN_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    set_steering_info_input_lights(TURN_SIGNAL_RIGHT);
  } else {
    set_steering_info_input_lights(TURN_SIGNAL_OFF);
  }
  // Read ADC of pin set by cruise control
  // Determine if it read an increase or decrease signal
  adc_read_converted(cc_address, &control_stalk_data);
  
  if (control_stalk_data > CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
      control_stalk_data < CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    // toggle second bit to 1
    set_steering_info_input_cc(CC_INCREASE_MASK | CC_INPUT);
  } else if (control_stalk_data >
                 CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
             control_stalk_data <
                 CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    // toggle first bit to 1
    set_steering_info_input_cc(CC_DECREASE_MASK | CC_INPUT);
  }
  
  
  if (notify_get(&notification) == STATUS_CODE_OK) {
    LOG_DEBUG("Notification status ok\n");
    while (event_from_notification(&notification, &STEERING_EVENT) == STATUS_CODE_INCOMPLETE) {
      LOG_DEBUG("Event Fetched\n");
      if (STEERING_EVENT == CC_TOGGLE_EVENT) {
        LOG_DEBUG("CC Toggle Event Returned\n");
        set_steering_info_input_cc(CC_TOGGLE_MASK | CC_INPUT);
      }
    }
  }else{
    LOG_DEBUG("CC Toggle Event FAILED\n");
  }
}
