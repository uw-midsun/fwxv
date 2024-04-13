#pragma once

#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "log.h"
#include "status.h"

#define VOLTAGE_TOLERANCE_MV 100
// turn signal voltages
#define TURN_LEFT_SIGNAL_VOLTAGE_MV 2100
#define TURN_RIGHT_SIGNAL_VOLTAGE_MV 1200
#define NEUTRAL_SIGNAL_VOLTAGE_MV 3300
// cruise control voltages
#define CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV 1600
#define CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV 775
#define CRUISE_CONTROl_STALK_NEUTRAL_VOLTAGE_MV 3300

#define TURN_SIGNAL_GPIO \
  { .port = GPIO_PORT_B, .pin = 7 }

#define CC_CHANGE_GPIO \
  { .port = GPIO_PORT_B, .pin = 6 }

#define CC_TOGGLE_GPIO \
  { .port = GPIO_PORT_B, .pin = 12 }

#define CC_TOGGLE_EVENT 0x0

// TurnSignal Values for each state
typedef enum TurnSignalState {
  TURN_SIGNAL_OFF = 0,
  TURN_SIGNAL_LEFT,
  TURN_SIGNAL_RIGHT,
} TurnSignalState;

// CruiseControl Mask Values for each state
typedef enum {
  CC_DECREASE_MASK = 0x01,
  CC_INCREASE_MASK = 0x02,
  CC_TOGGLE_MASK = 0x04,
} CruiseControlMask;
// Initializes the ADC input for the ctrl stalk as well as the gpio interrupt for the toggle
// Must be called before adc_init()
StatusCode steering_init();
// Reads the analog input, determines a light state, and sets the requisite value in the can message
// Also reads the toggle notification state and sets the requisite value in the can message
void steering_input();
