#pragma once

#include "can.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"
#include "task.h"

// TODO: figure out actual values for timeout
#define BMS_RESPONSE_TIMEOUT_MS 10000
#define MCI_RESPONSE_TIMEOUT_MS 12000

#define NUM_POWER_STATES 6

DECLARE_FSM(power_seq);
typedef enum PowerSeqStateId {
  POWER_STATE_OFF = 0,
  TRANSMIT_BMS_CLOSE_RELAYS,
  POWER_STATE_ON,
  TURN_ON_DRIVE_OUTPUTS,
  POWER_STATE_DRIVE,
  POWER_STATE_FAULT,
} PowerSeqStateId;

typedef struct {
  PowerSeqStateId target_state;
  PowerSeqStateId latest_state;
  TickType_t timer_start_ticks;
  uint8_t fault;
} PowerFsmContext;

StatusCode init_power_seq(void);
