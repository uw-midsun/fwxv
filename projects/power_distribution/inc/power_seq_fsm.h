#pragma once

#include "can.h"
#include "exported_enums.h"
#include "flash.h"
#include "fsm.h"
#include "log.h"
#include "persist.h"
#include "task.h"

// TODO: figure out actual values for timeout
#define BMS_RESPONSE_TIMEOUT_MS 3000
#define MCI_RESPONSE_TIMEOUT_MS 300
#define BPS_FAULT_FLASH_PAGE NUM_FLASH_PAGES - 1

#define NUM_POWER_STATES 4

typedef struct BpsStorage {
  uint16_t fault_bitset;
  uint16_t vehicle_speed;
} BpsStorage;

DECLARE_FSM(power_seq);
typedef enum PowerSeqStateId {
  POWER_STATE_OFF = 0,
  POWER_STATE_PRECHARGE,
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
