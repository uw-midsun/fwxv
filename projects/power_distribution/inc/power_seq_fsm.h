#pragma once

#include "can.h"
#include "fsm.h"
#include "log.h"
#include "task.h"

#define NUM_POWER_SEQ_STATES 5
#define NUM_POWER_SEQ_TRANSITIONS 9

DECLARE_FSM(power_seq);
typedef enum PowerSeqStateId {
  POWER_SEQ_INIT_STATE = 0,
  POWER_SEQ_EVERYTHING_ON,
  POWER_SEQ_DRIVER_CONTROLS,
  POWER_SEQ_MAIN_OPERATION,
  POWER_SEQ_AUX_POWER
} PowerSeqStateId;

StatusCode init_power_seq(void);
