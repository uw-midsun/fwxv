#pragma once
#include "fsm.h"
#include "log.h"
#include "semaphore.h"

typedef struct FSMStorage {
  StateId power_state;
  StatusCode error_code;
  Mutex mutex;
} FSMStorage;

void fsm_shared_mem_init(FSMStorage *storage); 
void fsm_shared_mem_set_power_state(FSMStorage *storage, StateId state);
StateId fsm_shared_mem_get_power_state(FSMStorage *storage);
void fsm_shared_mem_set_error_code(FSMStorage *storage, StatusCode code);
StatusCode fsm_shared_mem_get_error_code(FSMStorage *storage);

extern FSMStorage cc_storage;
