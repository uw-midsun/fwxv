#pragma once

#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "notify.h"
#include "task.h"

#define NUM_FSM1_STATES 3
#define NUM_FSM1_TRANSITIONS 3

DECLARE_FSM(fsm1);

typedef enum Fsm1StateId {
  FSM1_STATE_0 = 0,
  FSM1_STATE_1,
  FSM1_STATE_2,
} Fsm1StateId;

// Notifications to send to FSM 2
typedef enum Fsm1OutputEvent {
  FSM1_STATE_0_CMPL = 0,
  FSM1_STATE_1_CMPL,
  FSM1_STATE_2_CMPL,
} Fsm1OutputEvent;

typedef struct Fsm1Storage {
  GpioAddress start_btn_address;
} Fsm1Storage;

StatusCode init_fsm1(void);
