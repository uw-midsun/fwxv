#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "exported_enums.h"
#include "status.h"

#define MAX_VOLTAGE 4.2
#define MIN_VOLTAGE 4.0
#define MAX_CURRENT 27

typedef enum MainTaskBtnEvents {
  POWER_BUTTON_EVENT = 0,
  REGEN_BUTTON_EVENT,
  HAZARD_BUTTON_EVENT,
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
void update_drive_output(uint32_t notif);
