#pragma once

#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "power_fsm_sequence.h"
#include "task.h"

#define NUM_POWER_STATES 14
#define NUM_POWER_TRANSITIONS 40

DECLARE_FSM(power);

typedef enum PowerFsmStateId {
  POWER_FSM_STATE_OFF = 0,
  // -> MAIN Sequence
  POWER_FSM_CONFIRM_AUX_STATUS,
  POWER_FSM_SEND_PD_BMS,
  POWER_FSM_CONFIRM_BATTERY_STATUS,
  POWER_FSM_CLOSE_BATTERY_RELAYS,
  POWER_FSM_CONFIRM_DC_DC,
  POWER_FSM_TURN_ON_EVERYTHING,
  POWER_FSM_POWER_MAIN_COMPLETE,
  POWER_FSM_STATE_MAIN,
  // -> AUX Sequence
  // Confirm aux status
  // Turn on everything
  POWER_FSM_STATE_AUX,
  // -> FAULT Sequence (none)
  POWER_FSM_STATE_FAULT,
  // -> OFF Sequence
  POWER_FSM_DISCHARGE_PRECHARGE,
  POWER_FSM_TURN_OFF_EVERYTHING,
  POWER_FSM_OPEN_RELAYS
} PowerFsmStateId;

typedef struct PowerFsmContext {
  PowerFsmStateId latest_state;
  PowerFsmStateId target_state;
}   PowerFsmContext;

StatusCode init_power_fsm(void);
