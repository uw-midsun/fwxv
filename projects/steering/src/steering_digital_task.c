#include "steering_digital_task.h"

#define HORN_COMMAND g_tx_struct.digital_signal_horn_state
#define REGEN_BRAKE_COMMAND g_tx_struct.digital_signal_regen_brake_toggle_command
#define CRUISE_CONTROL_COMMAND g_tx_struct.digital_signal_cruise_control_command

static uint32_t notification = 0;
Event steering_event;

void run_steering_digital_task() {
  notify_get(&notification);

  while (event_from_notification(&notification, &steering_event)) {
    handle_state_change(steering_event);
  }
}

StatusCode handle_state_change(Event digital_input) {
  GpioState toggle_state;

  switch (digital_input) {
    case STEERING_INPUT_HORN_EVENT:

      set_digital_signal_horn_state(HORN_COMMAND ^ 1u);
      break;

    case STEERING_REGEN_BRAKE_EVENT:

      set_digital_signal_regen_brake_toggle_command(REGEN_BRAKE_COMMAND ^ 1u);
      break;

    case STEERING_CC_TOGGLE_EVENT:

      set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND ^
                                                DIGITAL_SIGNAL_CC_TOGGLE_MASK);
      break;

    case STEERING_INCREASE_SPEED_EVENT:
      toggle_state =
          gpio_get_state(&s_steering_lookup_table[STEERING_DIGITAL_INPUT_CC_TOGGLE], &toggle_state);

      if (toggle_state == GPIO_STATE_HIGH) {
        set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND ^
                                                  DIGITAL_SIGNAL_CC_INCREASE_MASK);
      }
      break;

    case STEERING_DECREASE_SPEED_EVENT:
      toggle_state =
          gpio_get_state(&s_steering_lookup_table[STEERING_DIGITAL_INPUT_CC_TOGGLE], &toggle_state);

      if (toggle_state == GPIO_STATE_HIGH) {
        set_digital_signal_cruise_control_command(CRUISE_CONTROL_COMMAND ^
                                                  DIGITAL_SIGNAL_CC_DECREASE_MASK);
      }
      break;

    default:
      return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

StatusCode steering_digital_input_init(void) {
  set_digital_signal_horn_state(0);
  set_digital_signal_cruise_control_command(0);
  set_digital_signal_regen_brake_toggle_command(0);

  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
    status_ok_or_return(
        gpio_init_pin(&s_steering_lookup_table[i], GPIO_INPUT_PULL_UP, GPIO_STATE_LOW));
  }
  return STATUS_CODE_OK;
}
