#pragma once

#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "status.h"
#include "steering_setters.h"

#define DIGITAL_SIGNAL_CC_TOGGLE_MASK 0x04
#define DIGITAL_SIGNAL_CC_INCREASE_MASK 0x02
#define DIGITAL_SIGNAL_CC_DECREASE_MASK 0x01

#define HORN_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 1 }

#define REGEN_BRAKE_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 4 }

#define CC_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 5 }

#define CC_INCREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 2 }

#define CC_DECREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 8 }

// enum may or may not be needed
typedef enum {
  STEERING_DIGITAL_INPUT_HORN = 0,
  STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED,
  STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED,
  NUM_STEERING_DIGITAL_INPUTS,
} SteeringInterfaceDigitalInput;

StatusCode steering_digital_input_init(void);
void run_steering_digital_task();
void handle_state_change(const int digital_input, const GpioState state);
