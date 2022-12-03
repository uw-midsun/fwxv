#pragma once

#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "task.h"

#define NUM_MCI_FSM_STATES 4
#define NUM_MCI_FSM_TRANSITIONS 9
#define CRUISE_MAX_SPEED 45  // TODO(devAdhiraj): update these with actual numbers
#define CRUISE_MIN_SPEED 12  // TODO(devAdhiraj): update these with actual numbers
DECLARE_FSM(mci_fsm);

typedef enum MciFsmStateId {
  MCI_FSM_STATE_OFF = 0,
  MCI_FSM_STATE_DRIVE,
  MCI_FSM_STATE_REVERSE,
  MCI_FSM_STATE_CRUISE,
} MciFsmStateId;

typedef enum DriveOutputEvent {
  MCI_FSM_GOTO_OFF = 0,
  MCI_FSM_GOTO_DRIVE,
  MCI_FSM_GOTO_REVERSE,
  MCI_FSM_GOTO_CRUISE,
} DriveOutputEvent;

StatusCode init_mci_fsm(void);
