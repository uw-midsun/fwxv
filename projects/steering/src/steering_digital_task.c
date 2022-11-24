#include "steering_digital_task.h"

#define CRUISE_CONTROL_COMMAND g_tx_struct.cruise_control_command_command

static GpioState GPIO_prev_state[NUM_STEERING_DIGITAL_INPUTS];

static GpioAddress s_steering_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
    [STEERING_DIGITAL_INPUT_HORN] = HORN_GPIO_ADDR, 
    [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = REGEN_BRAKE_TOGGLE_GPIO_ADDR,
    [STEERING_DIGITAL_INPUT_CC_TOGGLE] = CC_TOGGLE_GPIO_ADDR, 
    [STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED] = CC_INCREASE_SPEED_GPIO_ADDR, 
    [STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED] = CC_DECREASE_SPEED_GPIO_ADDR,
};

static void run_steering_digital_task() {
    GpioState state = GPIO_STATE_LOW;

    for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
        gpio_get_state(&s_steering_lookup_table[i], &state); 
        if (state != GPIO_prev_state[i]) {
            handle_state_change(i, &state);
        }   
    }
    
}

void handle_state_change(const int digital_input, const GpioState *state) {
    switch(digital_input) {
        case STEERING_DIGITAL_INPUT_HORN:

            if (state == GPIO_STATE_HIGH) {
                set_digital_signal_horn_state(1);
            } else {
                set_digital_signal_horn_state(0);
                set_cruise_control_command_command(0);
            }
            break;

        case STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE:

            if (state == GPIO_STATE_HIGH) {
                set_digital_signal_regen_brake_toggle_command(1);
            } else {
                set_digital_signal_regen_brake_toggle_command(0);
            }
            break;

        case STEERING_DIGITAL_INPUT_CC_TOGGLE:
            if (state == GPIO_STATE_HIGH) {
                set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND | DIGITAL_SIGNAL_CC_TOGGLE_MASK);
            } else {
                set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND ^ DIGITAL_SIGNAL_CC_TOGGLE_MASK);
            }
            break;

        case STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED:
            if (state == GPIO_STATE_HIGH) {
                set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND | DIGITAL_SIGNAL_CC_INCREASE_MASK);
            } else {
                set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND ^ DIGITAL_SIGNAL_CC_INCREASE_MASK);
            }
            break;

        case STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED:
            if (state == GPIO_STATE_HIGH) {
                set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND | DIGITAL_SIGNAL_CC_DECREASE_MASK);
            } else {
                set_digital_signal_cruise_contorl_command(CRUISE_CONTROL_COMMAND ^ DIGITAL_SIGNAL_CC_DECREASE_MASK);
            }
            break;

        default: 
            return;
    }
}

StatusCode steering_digital_input_init(void) {

    GpioSettings digital_input_settings = {
        .direction = GPIO_DIR_IN, 
        .state = GPIO_STATE_LOW, 
        .resistor = GPIO_RES_NONE, 
        .alt_function = GPIO_ALTFN_NONE,
    }; 

    GpioState state = GPIO_STATE_LOW;

    for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
        status_ok_or_return(gpio_init_pin(&s_steering_lookup_table[i], &digital_input_settings)); 
        GPIO_prev_state[i] = gpio_get_state(&s_steering_lookup_table[i], &state);
    }
    return STATUS_CODE_OK;
}

