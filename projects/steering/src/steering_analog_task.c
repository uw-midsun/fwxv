#include "steering_analog_task.h"


#define CONTROL_STALK_STATE g_tx_struct.steering_info_analog_input

StatusCode steering_analog_adc_init()
{
    gpio_init_pin(&control_stalk_GPIO, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
    adc_add_channel(control_stalk_GPIO);
    return STATUS_CODE_OK;
}

void run_steering_analog_task() {
  uint16_t control_stalk_data;

    adc_read_converted(control_stalk_GPIO, &control_stalk_data);


    if (control_stalk_data > STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV && control_stalk_data < STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
      set_steering_info_analog_input(STEERING_LIGHT_LEFT);
    } else if (control_stalk_data > STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV && control_stalk_data < STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
      set_steering_info_analog_input(STEERING_LIGHT_RIGHT);
    }
    else{//default will be light type = right, state = off
      set_steering_info_analog_input(STEERING_LIGHT_OFF);
    }
}