#pragma once
// Wrapper library for all mutex and semaphore usage
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "status.h"

#define BLOCK_INDEFINITELY UINT16_MAX

// Mutex handle is used to access Mutex
// Mutex Objects must be declared statically 
typedef struct Mutex {
  SemaphoreHandle_t handle;
  StaticSemaphore_t buffer; 
} Mutex;

// Initializes a mutex using provided static entity
// Returns STATUS_CODE_OK on success, STATUS_CODE_UNINITIALIZED on failure
StatusCode mutex_init(Mutex *mutex);

// Locks a Mutex. The locking task is responsible for unlocking the mutex
// Waits ms_to_wait milliseconds if mutex is already locked, then times out
// Using BLOCK_INDEFINITELY will cause this method to wait forever on mutex becoming available
// Returns STATUS_CODE_OK on success, STATUS_CODE_TIMEOUT on timeout
StatusCode mutex_lock(Mutex *mutex, uint16_t ms_to_wait);

// Unlocks a Mutex. The task which locks the mutex MUST also be the one to unlock it
// Caller specifies whether the method is being called from a task or interrupt handler
// higher_priority_task_woken informs the caller whether a higher priority task has become unblocked
// Returns STATUS_CODE_OK on success, STATUS_CODE_INTERNAL_ERROR on failure
StatusCode mutex_unlock(Mutex *mutex);
