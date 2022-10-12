#pragma once

#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "notify.h"
#include "task.h"

#define NUM_MCI_FSM_STATES 4
#define NUM_MCI_FSM_TRANSITIONS 9
#define CRUISE_MAX_SPEED 45  // TODO: update these with actual numbers
#define CRUISE_MIN_SPEED 12  // TODO: update these with actual numbers
DECLARE_FSM(mci_fsm);

typedef enum MciFsmStateId {
  MCI_FSM_STATE_OFF = 0,
  MCI_FSM_STATE_DRIVE,
  MCI_FSM_STATE_REVERSE,
  MCI_FSM_STATE_CRUISE,
} MciFsmStateId;

// Events that MCI FSM Recieves?
typedef enum MciFsmOutputEvent {
  MCI_FSM_GOTO_OFF = 0,
  MCI_FSM_GOTO_DRIVE,
  MCI_FSM_GOTO_REVERSE,
  MCI_FSM_GOTO_CRUISE,
} MciFsmOutputEvent;

typedef struct MciFsmStorage {
  // updated every 200ms
  uint8_t velocity;  // (m/s)
  int16_t rpm;
  uint16_t bus_voltage_v;
  uint16_t bus_current_a;
  uint8_t mc_limit_bitset;
  uint8_t mc_error_bitset;
  // updated every second
  int8_t motor_temp_c;
  int8_t heatsink_temp_c;
  int8_t dsp_temp_c;
  float motor_temp;      // Celsius
  float heat_sink_temp;  // Celsius
} MciFsmStorage;

StatusCode init_mci_fsm(void);
