#include "fsm_shared_mem.h"

// mutex protect the FSMStorage using the semaphore.h library

// initialize the FSMStorage struct
void fsm_shared_mem_init(FSMStorage *storage) {
  if (mutex_init(&storage->mutex) != STATUS_CODE_OK) {
    LOG_DEBUG("mutex init failed\n");
    return;
  }
  // power_state hard coded to POWER_FSM_STATE_OFF
  storage->power_state = 0;
  storage->error_code = STATUS_CODE_OK;
}

// set power_state
void fsm_shared_mem_set_power_state(FSMStorage *storage, StateId state) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  storage->power_state = state;
  mutex_unlock(&storage->mutex);
}

// read power_state from FSMStorage
StateId fsm_shared_mem_get_power_state(FSMStorage *storage) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  StateId state = storage->power_state;
  mutex_unlock(&storage->mutex);
  return state;
}

// set error_code
void fsm_shared_mem_set_error_code(FSMStorage *storage, StatusCode code) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  storage->error_code = code;
  mutex_unlock(&storage->mutex);
}

// read error_code from FSMStorage
StatusCode fsm_shared_mem_get_error_code(FSMStorage *storage) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  StatusCode error = storage->error_code;
  mutex_unlock(&storage->mutex);
  return error;
}
