#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "exported_enums.h"
#include "status.h"

// TODO (Aryan): validate these numbers
#define MAX_VOLTAGE 42000
#define MIN_VOLTAGE 40000
#define MAX_CURRENT 58.2

typedef enum MainTaskBtnEvents {
  POWER_BUTTON_EVENT = 0,
  REGEN_BUTTON_EVENT,
  HAZARD_BUTTON_EVENT,
  CC_TOGGLE_EVENT,
} MainTaskBtnEvents;

// Initializes outputs for dashboard. Must be called after scheduler start
StatusCode dashboard_init();

// Updates indicators based on notification value and CAN messages
void update_indicators(uint32_t notif);

// Updates cruise control values based on inputs from steering
void monitor_cruise_control(void);

// Update 7-seg displays
void update_displays(void);

// Update drive output based
void update_drive_output();
