#include "fsm_shared_mem.h"

// mutex protect the FSMStorage using the semaphore.h library

// initialize the FSMStorage struct
void fsm_shared_mem_init(FSMStorage *storage) {
  if (mutex_init(&storage->mutex) != STATUS_CODE_OK) {
    LOG_DEBUG("mutex init failed\n");
    return;
  }
  // storage->power_state hard coded to POWER_FSM_STATE_OFF
  storage->power_state = 0;
  storage->power_error_code = STATUS_CODE_OK;
  storage->drive_error_code = STATUS_CODE_OK;
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

// set power error_code
void fsm_shared_mem_set_power_error_code(FSMStorage *storage, StatusCode code) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  storage->power_error_code = code;
  mutex_unlock(&storage->mutex);
}

// read power error_code from FSMStorage
StatusCode fsm_shared_mem_get_power_error_code(FSMStorage *storage) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  StatusCode error = storage->power_error_code;
  mutex_unlock(&storage->mutex);
  return error;
}

// set drive error_code
void fsm_shared_mem_set_drive_error_code(FSMStorage *storage, StatusCode code) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  storage->drive_error_code = code;
  mutex_unlock(&storage->mutex);
}

// read drive error_code from FSMStorage
StatusCode fsm_shared_mem_get_drive_error_code(FSMStorage *storage) {
  mutex_lock(&storage->mutex, BLOCK_INDEFINITELY);
  StatusCode error = storage->drive_error_code;
  mutex_unlock(&storage->mutex);
  return error;
}
