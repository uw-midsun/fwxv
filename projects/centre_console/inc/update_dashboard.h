#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "status.h"
#include "exported_enums.h"

typedef enum MainTaskBtnEvents {
  POWER_BUTTON_EVENT = 0,
  REGEN_BUTTON_EVENT,
  HAZARD_BUTTON_EVENT,
} MainTaskBtnEvents;

void update_indicators(void);
void update_displays(void);
void monitor_cruise_control();
