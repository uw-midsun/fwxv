#pragma once
#include "fsm.h"
#include "fsm_shared_mem.h"
#include "log.h"
#include "semaphore.h"

typedef struct FSMStorage {
  StateId power_state;
  StatusCode power_error_code;
  StatusCode drive_error_code;
  Mutex mutex;
} FSMStorage;

// Initialize fsm shared memory
void fsm_shared_mem_init();

// Set Power FSM state in shared memory
void fsm_shared_mem_set_power_state(StateId state);

// Get Power FSM state
StateId fsm_shared_mem_get_power_state();

// Set Power FSM Error Code
void fsm_shared_mem_set_power_error_code(StatusCode code);

// Get Power FSM Error Code
StatusCode fsm_shared_mem_get_power_error_code();

// Set Drive FSM Error Code
void fsm_shared_mem_set_drive_error_code(StatusCode code);

// Get Drive FSM Error Code
StatusCode fsm_shared_mem_get_drive_error_code();
