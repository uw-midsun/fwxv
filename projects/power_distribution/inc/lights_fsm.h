#pragma once

#include "can.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "task.h"

#define NUM_LIGHTS_STATES 4
#define NUM_LIGHTS_TRANSITIONS 10

#define RIGHT_LIGHT_ADDR \
  { .port = GPIO_PORT_A, .pin = 13 }

#define LEFT_LIGHT_ADDR \
  { .port = GPIO_PORT_A, .pin = 12 }

DECLARE_FSM(lights);

// Light events matches CAN message defs from steering analog
typedef enum LightsStateId {
  INIT_STATE = 0,
  LEFT_SIGNAL,
  RIGHT_SIGNAL,
  HAZARD,
  NUM_STATES,
} LightsStateId;

// Light type to be used with SYSTEM_CAN_MESSAGE_LIGHTS_STATE message
typedef enum EELightType {
  EE_LIGHT_TYPE_STROBE = 0,
  EE_LIGHT_TYPE_SIGNAL_RIGHT,
  EE_LIGHT_TYPE_SIGNAL_LEFT,
  EE_LIGHT_TYPE_SIGNAL_HAZARD,
  NUM_EE_LIGHT_TYPES,
} EELightType;

// Light state to be used with a SYSTEM_CAN_MESSAGE_LIGHTS message
typedef enum EELightState {
  EE_LIGHT_STATE_OFF = 0,
  EE_LIGHT_STATE_ON,
  NUM_EE_LIGHT_STATES,
} EELightState;

StatusCode init_lights(void);
