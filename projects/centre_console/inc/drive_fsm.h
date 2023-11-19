#pragma once

#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "notify.h"
#include "task.h"

#define NUM_DRIVE_STATES 3
#define NUM_DRIVE_TRANSITIONS 4

#define POWER_FSM_STATE_OFF 0   // will be changed later
#define POWER_FSM_STATE_ON      // will be changed later
#define POWER_FSM_STATE_MAIN 8  // will be changed later

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

#define NEUTRAL_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 6 }

#define DRIVE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 9 }

#define REVERSE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 7 }

StateId get_drive_state(void);

StatusCode init_drive_fsm(void);
