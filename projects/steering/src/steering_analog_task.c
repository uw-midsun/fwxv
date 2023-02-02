#include "steering_analog_task.h"

#define DRL_ID g_tx_struct.DRL_id
#define DRL_STATE g_tx_struct.DRL_state
#define CONTROL_STALK_ID g_tx_struct.control_stalk_id
#define CONTROL_STALK_STATE g_tx_struct.control_stalk_state

StatusCode steering_analog_adc_init()
{
    GpioSettings stalk_settings = {
        GPIO_DIR_IN,
        GPIO_STATE_LOW,
        GPIO_RES_NONE,
        GPIO_ALTFN_ANALOG,
    }

    gpio_init_pin(&control_stalk, &stalk_settings);

    adc_init(ADC_MODE_SINGLE);



    adc_add_channel(control_stalk_GPIO);
}

TASK(run_steering_analog_task, TASK_MIN_STACK_SIZE) {
  int control_stalk_data;

    adc_read_converted(control_stalk_GPIO, &control_stalk_data);


    if (control_stalk_data > STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV && control_stalk_data < STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
      set_analog_input(STEERING_LIGHT_LEFT);
    } else if (control_stalk_data > STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV - VOLTAGE_TOLERANCE_MV && control_stalk_data < STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV + VOLTAGE_TOLERANCE_MV) {
      set_analog_input(STEERING_LIGHT_RIGHT);
    }
    else{//default will be light type = right, state = off
      set_analog_input(STEERING_LIGHT_OFF);
    }
}