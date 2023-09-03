#pragma once
// Wraps the LTC AFE module and handles all the sequencing.
// Requires LTC AFE, soft timers to be initialized.
//
#include "fsm.h"
#include "ltc_afe.h"

#define LTC_AFE_FSM_CELL_CONV_DELAY_MS 10
#define LTC_AFE_FSM_AUX_CONV_DELAY_MS 6
// Maximum number of retry attempts to read cell/aux data once triggered
#define LTC_AFE_FSM_MAX_RETRY_COUNT 3

#define NUM_LTC_AFE_FSM_STATES 6
#define NUM_LTC_AFE_FSM_TRANSITIONS 14

DECLARE_FSM(ltc_afe_fsm);

typedef enum LtcAfeFsmStateId {
  LTC_AFE_IDLE = 0,
  LTC_AFE_TRIGGER_CELL_CONV,
  LTC_AFE_READ_CELLS,
  LTC_AFE_TRIGGER_AUX_CONV,
  LTC_AFE_READ_AUX,
  LTC_AFE_AUX_COMPLETE,
  LTC_AFE_FAULT
} LtcAfeFsmStateId;

// We can raise a fault using this when transitioning to LTC_AFE_FAULT to identify where it came
// from
typedef enum {
  LTC_AFE_FSM_FAULT_TRIGGER_CELL_CONV = 0,
  LTC_AFE_FSM_FAULT_READ_ALL_CELLS,
  LTC_AFE_FSM_FAULT_TRIGGER_AUX_CONV,
  LTC_AFE_FSM_FAULT_READ_AUX,
  NUM_LTC_AFE_FSM_FAULTS
} LtcAfeFsmFault;

typedef enum {
  LTC_TRIGGER_CELL_EVENT_START = 0,
  LTC_TRIGGER_AUX_EVENT_START,
  NUM_LTC_AFE_ADC_MODES
} LtcAfeOutputEvent;

StatusCode ltc_afe_fsm_init(Fsm *fsm, LtcAfeStorage *afe);
