#pragma once

#include "fsm.h"

#include "delay.h"
#include "log.h"
#include "notify.h"
#include "task.h"
#include "gpio.h"

#define NUM_FSM2_STATES 3
#define NUM_FSM2_TRANSITIONS 7

DECLARE_FSM(fsm2);

typedef enum Fsm2StateId {
  FSM2_STATE_0 = 0,
  FSM2_STATE_1,
  FSM2_STATE_2,
} Fsm2StateId;

// Notifications to send to FSM 1
typedef enum Fsm2OutputEvent {
  FSM2_STATE_0_CMPL,
  FSM2_STATE_1_CMPL,
  FSM2_STATE_2_CMPL,
} Fsm2OutputEvent;
  

StatusCode init_fsm2(void);
