#pragma once

#include "delay.h"
#include "fsm.h"
#include "fsm_shared_mem.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "notify.h"
#include "task.h"

#define NUM_DRIVE_STATES 4
#define NUM_DRIVE_TRANSITIONS 8

#define BEGIN_PRECHARGE_SIGNAL 2
#define PRECHARGE_STATE_COMPLETE 2
#define NUMBER_OF_CYCLES_TO_WAIT 10

DECLARE_FSM(drive);

typedef enum driveState {
  NEUTRAL = 0,
  DRIVE,
  REVERSE,
  DO_PRECHARGE,  // turn on precharge and get ack
} driveState;

typedef enum driveButtons {
  NEUTRAL_BUTTON = 0,
  DRIVE_BUTTON,
  REVERSE_BUTTON,
  NUM_DRIVE_FSM_BUTTONS,
} driveButtons;

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

StatusCode init_drive_fsm(void);
