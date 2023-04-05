#include "steering_analog_task.h"

#define CONTROL_STALK_STATE g_tx_struct.steering_info_analog_input

static const GpioAddress s_ctrl_stk_address = CTRL_STALK_GPIO;

StatusCode steering_analog_adc_init() {
  gpio_init_pin(&s_ctrl_stk_address, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  adc_add_channel(s_ctrl_stk_address);
  return STATUS_CODE_OK;
}

void steering_analog_input() {
  uint16_t control_stalk_data;
  uint8_t channel;
  // Read ADC of pin set by steering control
  // Determine if it's a right, left or off signal
  adc_read_converted(s_ctrl_stk_address, &control_stalk_data);
  if (control_stalk_data > STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
      control_stalk_data < STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    set_steering_info_analog_input(STEERING_LIGHT_LEFT);
  } else if (control_stalk_data >
                 STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV &&
             control_stalk_data <
                 STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
    set_steering_info_analog_input(STEERING_LIGHT_RIGHT);
  } else {
    set_steering_info_analog_input(STEERING_LIGHT_OFF);
  }
}
