#pragma once 

#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "notify.h"
#include "task.h"

#define NUM_DRIVE_STATES 6
#define NUM_DRIVE_TRANSITIONS 11

DECLARE_FSM(drive_fsm);

typedef enum driveState {
  NEUTRAL = 0,
  DRIVE,
  REVERSE,
  GET_PRECHARGE,  // gets precharge state
  DO_PRECHARGE,   // turn on precharge and get ack
  TRANSMIT, // send drive state (NEUTRAL, DRIVE, or REVERSE) to MCI
} driveState;

StatusCode init_drive_fsm(void);
