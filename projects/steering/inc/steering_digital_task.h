#pragma once

#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "notify.h"
#include "status.h"
#include "steering_setters.h"

// List of gpio inputs controlled by steering digital
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

// Masks for each respective CAN message signal bit in digital_input
// All digital inputs are contained within a 1Byte signal
typedef enum {
  DIGITAL_SIGNAL_CC_DECREASE_MASK = 0x01,
  DIGITAL_SIGNAL_CC_INCREASE_MASK = 0x02,
  DIGITAL_SIGNAL_CC_TOGGLE_MASK = 0x04,
  DIGITAL_SIGNAL_REGEN_BRAKE_MASK = 0x08,
  DIGITAL_SIGNAL_HORN_MASK = 0x10,
} SteeringMask;

typedef enum {
  STEERING_DIGITAL_INPUT_HORN = 0,
  STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED,
  STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED,
  NUM_STEERING_DIGITAL_INPUTS,
} SteeringInterfaceDigitalInput;

// Sets values corresponding to possible digital input events
typedef enum {
  STEERING_INPUT_HORN_EVENT = 0,
  STEERING_REGEN_BRAKE_EVENT,
  STEERING_CC_TOGGLE_EVENT,
  STEERING_CC_INCREASE_SPEED_EVENT,
  STEERING_CC_DECREASE_SPEED_EVENT,
  NUM_STEERING_EVENTS,
} SteeringDigitalEvent;

// Initializes all digital input GPIO's'and GPIO interrupts
// Return STATUS_CODE_OK if initialization successful
StatusCode steering_digital_input_init(Task *task);

// Parses digital_input Event to send the appropriate CAN message signal
// Returns STATUS_CODE_OK if digital_input is processed successfully, and
// STATUS_CODE_INVALID_ARGS if an invalid Event is received
StatusCode handle_state_change(Event digital_input);

// Gets a notification and calls handle_state_change() to process events associated with it
void steering_digital_input(void);
