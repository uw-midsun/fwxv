#pragma once

#include "delay.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "notify.h"
#include "task.h"

#define NUM_DRIVE_STATES 3
#define NUM_DRIVE_TRANSITIONS 4

DECLARE_FSM(drive);

typedef enum driveState {
  NEUTRAL = 0,
  DRIVE,
  REVERSE,
} driveState;

typedef enum driveEvents {
  NEUTRAL_BUTTON_EVENT = 0,
  DRIVE_BUTTON_EVENT,
  REVERSE_BUTTON_EVENT,
  NUM_DRIVE_FSM_EVENTS,
} driveEvents;

StateId get_drive_state(void);

StatusCode init_drive_fsm(void);
