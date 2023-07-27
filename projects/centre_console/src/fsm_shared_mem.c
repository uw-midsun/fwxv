#include "fsm_shared_mem.h"

// mutex protect the FSMStorage using the semaphore.h library

#define MUTEX_BLOCK_TIME_MS 100

static FSMStorage cc_storage = { 0 };

// initialize the FSMStorage struct
void fsm_shared_mem_init() {
  if (mutex_init(&cc_storage.mutex) != STATUS_CODE_OK) {
    LOG_DEBUG("mutex init failed\n");
    return;
  }
  // cc_storage.power_state hard coded to POWER_FSM_STATE_OFF
  cc_storage.power_state = 0;
  cc_storage.power_error_code = STATUS_CODE_OK;
  cc_storage.drive_error_code = STATUS_CODE_OK;
}

// set power_state
void fsm_shared_mem_set_power_state(StateId state) {
  mutex_lock(&cc_storage.mutex, MUTEX_BLOCK_TIME_MS);
  cc_storage.power_state = state;
  mutex_unlock(&cc_storage.mutex);
}

// read power_state from FSMStorage
StateId fsm_shared_mem_get_power_state() {
  mutex_lock(&cc_storage.mutex, MUTEX_BLOCK_TIME_MS);
  StateId state = cc_storage.power_state;
  mutex_unlock(&cc_storage.mutex);
  return state;
}

// set power error_code
void fsm_shared_mem_set_power_error_code(StatusCode code) {
  mutex_lock(&cc_storage.mutex, MUTEX_BLOCK_TIME_MS);
  cc_storage.power_error_code = code;
  mutex_unlock(&cc_storage.mutex);
}

// read power error_code from FSMStorage
StatusCode fsm_shared_mem_get_power_error_code() {
  mutex_lock(&cc_storage.mutex, MUTEX_BLOCK_TIME_MS);
  StatusCode error = cc_storage.power_error_code;
  mutex_unlock(&cc_storage.mutex);
  return error;
}

// set drive error_code
void fsm_shared_mem_set_drive_error_code(StatusCode code) {
  mutex_lock(&cc_storage.mutex, MUTEX_BLOCK_TIME_MS);
  cc_storage.drive_error_code = code;
  mutex_unlock(&cc_storage.mutex);
}

// read drive error_code from FSMStorage
StatusCode fsm_shared_mem_get_drive_error_code() {
  mutex_lock(&cc_storage.mutex, MUTEX_BLOCK_TIME_MS);
  StatusCode error = cc_storage.drive_error_code;
  mutex_unlock(&cc_storage.mutex);
  return error;
}
