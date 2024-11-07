#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "exported_enums.h"
#include "status.h"

// TODO (Aryan): validate these numbers
#define MAX_VOLTAGE 42000
#define MIN_VOLTAGE 40000
#define MAX_CURRENT 58.2

#define MATH_PI 3.14159
#define WHEEL_DIAMETER_CM 55.84
#define CM_TO_INCHES 2.54
#define MILES_TO_INCHES 63360

typedef enum MainTaskBtnEvents {
  POWER_BUTTON_EVENT = 0,
  REGEN_BUTTON_EVENT,
  HAZARD_BUTTON_EVENT,
  CC_TOGGLE_EVENT,
} MainTaskBtnEvents;

// Initializes outputs for dashboard. Must be called after scheduler start
StatusCode dashboard_init();

// Initialized the display. Must be caleld before scheduler start
StatusCode display_init(void);

// Updates indicators based on notification value and CAN messages
void update_indicators(uint32_t notif);

// Updates cruise control values based on inputs from steering
void monitor_cruise_control(void);

// Update drive output based
void update_drive_output();
