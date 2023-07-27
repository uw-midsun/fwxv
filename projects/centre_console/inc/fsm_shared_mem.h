#pragma once
#include "fsm.h"
#include "log.h"
#include "semaphore.h"

typedef struct FSMStorage {
  StateId power_state;
  StatusCode power_error_code;
  StatusCode drive_error_code;
  Mutex mutex;
} FSMStorage;

void fsm_shared_mem_init();
void fsm_shared_mem_set_power_state(StateId state);
StateId fsm_shared_mem_get_power_state();
void fsm_shared_mem_set_power_error_code(StatusCode code);
StatusCode fsm_shared_mem_get_power_error_code();
void fsm_shared_mem_set_drive_error_code(StatusCode code);
StatusCode fsm_shared_mem_get_drive_error_code();
