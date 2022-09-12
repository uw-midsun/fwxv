#include "semaphore.h"

#include <stdio.h>

StatusCode mutex_init(Mutex *mutex) {
  mutex->handle = xSemaphoreCreateMutexStatic(&mutex->buffer);
  if (mutex->handle == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  } else {
    return STATUS_CODE_OK;
  }
}

StatusCode mutex_lock(Mutex *mutex, uint16_t ms_to_wait) {
  TickType_t ticks_to_wait;
  if (mutex == NULL || mutex->handle == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (ms_to_wait == BLOCK_INDEFINITELY) {
    ticks_to_wait = portMAX_DELAY;
  } else {
    ticks_to_wait = pdMS_TO_TICKS(ms_to_wait);
  }
  if (xSemaphoreTake(mutex->handle, ticks_to_wait) == pdFALSE) {
    return STATUS_CODE_TIMEOUT;
  }
  return STATUS_CODE_OK;
}

StatusCode mutex_unlock(Mutex *mutex) {
  if (mutex == NULL || mutex->handle == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (xSemaphoreGive(mutex->handle) == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

StatusCode sem_init(Semaphore *sem, uint32_t max_count, uint32_t initial_count) {
  if (sem == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  sem->handle = xSemaphoreCreateCountingStatic(max_count, initial_count, &sem->buffer);
  if (sem->handle == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  } else {
    return STATUS_CODE_OK;
  }
}

StatusCode sem_wait(Semaphore *sem, uint32_t timeout_ms) {
  if (sem == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (xSemaphoreTake(sem->handle, timeout_ms) == pdFALSE) {
    return STATUS_CODE_TIMEOUT;
  }
  return STATUS_CODE_OK;
}

StatusCode sem_post(Semaphore *sem) {
  if (sem == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (xSemaphoreGive(sem->handle) == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

uint32_t sem_num_items(Semaphore *sem) {
  if (sem == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  UBaseType_t count = uxSemaphoreGetCount(sem->handle);
  return count;
}
