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

//https://github.com/uw-midsun/hardware/blob/master/MSXII_SteeringWheelInterfaceBoard/Project%20Outputs%20for%20MSXII_SteeringWheelInterfaceBoard/MSXII_SteeringWheelInterfaceBoard_3.1.PDF
static GpioAddress control_stalk_GPIO = { .port = GPIO_PORT_A, .pin = 3};

void run_steering_analog_task();
StatusCode steering_analog_adc_init();

typedef enum SteeringLightState{
  STEERING_LIGHT_OFF = 0,
  STEERING_LIGHT_LEFT,
  STEERING_LIGHT_RIGHT,
} SteeringLightState;

typedef enum EELightState {
  EE_LIGHT_STATE_OFF = 0,
  EE_LIGHT_STATE_ON,
  NUM_EE_LIGHT_STATES,
} EELightState;

