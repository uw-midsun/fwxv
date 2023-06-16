#pragma once

#include "delay.h"
#include "fsm.h"
#include "fsm_shared_mem.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "notify.h"
#include "task.h"

#define NUM_DRIVE_STATES 5
#define NUM_DRIVE_TRANSITIONS 9

DECLARE_FSM(drive);

typedef enum driveState {
  NEUTRAL = 0,
  DRIVE,
  REVERSE,
  DO_PRECHARGE,  // turn on precharge and get ack
  TRANSMIT,      // send drive state (NEUTRAL, DRIVE, or REVERSE) to MCI
} driveState;

typedef enum driveButtons {
  NEUTRAL_BUTTON,
  DRIVE_BUTTON,
  REVERSE_BUTTON,
  NUM_DRIVE_FSM_BUTTONS,
} driveButtons;

typedef enum driveEvents {
  NEUTRAL_BUTTON_EVENT,
  DRIVE_BUTTON_EVENT,
  REVERSE_BUTTON_EVENT,
  NUM_DRIVE_FSM_EVENTS,
} driveEvents;

// this needs to be deleted. Just here as a placeholder
#define FAKE_NEUTRAL_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 1 }
  
#define FAKE_DRIVE_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 2 }

#define FAKE_REVERSE_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 3 }

typedef struct DriveStorage {
  StateId state;  // NEUTRAL will be used as a default state
} DriveStorage;

extern DriveStorage drive_storage;

StatusCode init_drive_fsm(void);
void prv_set_or_get_error_state();
