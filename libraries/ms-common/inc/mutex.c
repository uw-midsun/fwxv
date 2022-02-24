#include "mutex.h"

StatusCode mutex_init(Mutex *mutex) {
  mutex->handle = xSemaphoreCreateMutexStatic(&mutex->buffer);
  if(mutex->handle == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  } else {
    return STATUS_CODE_OK;
  }

StatusCode mutex_lock(Mutex *mutex, uint16_t ms_to_wait) {
  TickType_t ticks_to_wait;
  if(ms_to_wait == BLOCK_INDEFINITELY) {
    ticks_to_wait = portMAX_DELAY;
  } else {
    ticks_to_wait = pdMS_TO_TICKS(ms_to_wait); 
  }
  if (xSemaphoreTake(mutex->handle, ticks_to_wait) == pdFALSE) {
    return STATUS_CODE_TIMEOUT;
  }
  return STATUS_CODE_OK;
}

StatusCode mutex_unlock(Mutex *mutex, MutexCaller caller) {
  bool result;
  if(caller == MUTEX_CALLER_ISR) {
    // Use ISR specific method
    result = xSemaphoreGiveFromISR(mutex->handle);
  } else {
    result = xSemaphoreGive(mutex->handle);
  }
  if (result == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}
