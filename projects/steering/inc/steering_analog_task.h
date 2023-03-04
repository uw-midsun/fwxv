#pragma once

#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "log.h"
#include "status.h"
#include "steering_setters.h"

#define VOLTAGE_TOLERANCE_MV 20
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV 3025
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV 2984
#define CTRL_STALK_GPIO \
  { .port = GPIO_PORT_A, .pin = 3 }

typedef enum SteeringLightState {
  STEERING_LIGHT_OFF = 0,
  STEERING_LIGHT_LEFT,
  STEERING_LIGHT_RIGHT,
} SteeringLightState;

// Initializes the ADC input for the ctrl stalk
// Must be called before adc_init()
StatusCode steering_analog_adc_init();

// Reads the analog input, determines a light state, and sets the requisite value in the can message
void steering_analog_input();
