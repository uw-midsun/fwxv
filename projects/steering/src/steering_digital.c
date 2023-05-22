#include "steering_digital_task.h"

#define DIGITAL_INPUT g_tx_struct.steering_info_digital_input

static GpioAddress s_steering_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = HORN_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = REGEN_BRAKE_TOGGLE_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = CC_TOGGLE_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED] = CC_INCREASE_SPEED_GPIO_ADDR,
  [STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED] = CC_DECREASE_SPEED_GPIO_ADDR,
};

static Event s_steering_event_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = STEERING_INPUT_HORN_EVENT,
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = STEERING_REGEN_BRAKE_EVENT,
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = STEERING_CC_TOGGLE_EVENT,
  [STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED] = STEERING_CC_INCREASE_SPEED_EVENT,
  [STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED] = STEERING_CC_DECREASE_SPEED_EVENT,
};

static uint32_t notification = 0;
static Event steering_event;

void steering_digital_input() {
  set_steering_info_digital_input(
      DIGITAL_INPUT & ~(DIGITAL_SIGNAL_CC_DECREASE_MASK | DIGITAL_SIGNAL_CC_INCREASE_MASK));

  if (notify_get(&notification) == STATUS_CODE_OK) {
    while (event_from_notification(&notification, &steering_event) == STATUS_CODE_INCOMPLETE) {
      handle_state_change(steering_event);
    }
  }
}

StatusCode handle_state_change(Event digital_input) {
  switch (digital_input) {
    case STEERING_INPUT_HORN_EVENT:

      set_steering_info_digital_input(DIGITAL_INPUT ^ DIGITAL_SIGNAL_HORN_MASK);
      break;
    case STEERING_REGEN_BRAKE_EVENT:

      set_steering_info_digital_input(DIGITAL_INPUT ^ DIGITAL_SIGNAL_REGEN_BRAKE_MASK);
      break;
    case STEERING_CC_TOGGLE_EVENT:

      set_steering_info_digital_input(DIGITAL_INPUT ^ DIGITAL_SIGNAL_CC_TOGGLE_MASK);
      break;
    case STEERING_CC_INCREASE_SPEED_EVENT:

      set_steering_info_digital_input(DIGITAL_INPUT | DIGITAL_SIGNAL_CC_INCREASE_MASK);
      break;
    case STEERING_CC_DECREASE_SPEED_EVENT:

      set_steering_info_digital_input(DIGITAL_INPUT | DIGITAL_SIGNAL_CC_DECREASE_MASK);
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

StatusCode steering_digital_input_init(Task *task) {
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_FALLING,
  };

  InterruptSettings horn_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_RISING_FALLING,
  };

  set_steering_info_digital_input(0);
  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
    status_ok_or_return(
        gpio_init_pin(&s_steering_lookup_table[i], GPIO_INPUT_PULL_UP, GPIO_STATE_LOW));

    if (i == STEERING_DIGITAL_INPUT_HORN) {
      gpio_it_register_interrupt(&s_steering_lookup_table[i], &horn_settings,
                                 s_steering_event_lookup_table[i], task);
    } else {
      gpio_it_register_interrupt(&s_steering_lookup_table[i], &it_settings,
                                 s_steering_event_lookup_table[i], task);
    }
  }
  return STATUS_CODE_OK;
}
