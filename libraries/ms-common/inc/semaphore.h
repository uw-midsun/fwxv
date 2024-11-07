#pragma once
// Wrapper library for all mutex and semaphore usage
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "status.h"

#define BLOCK_INDEFINITELY UINT16_MAX

// Mutex Objects must be declared statically
// Mutexes should NOT be used from ISRs (only sems)
typedef struct Semaphore {
  SemaphoreHandle_t handle;
  StaticSemaphore_t buffer;
} Semaphore;

typedef Semaphore Mutex;

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

// Initializes counting semaphore with max and initial count
// Returns STATUS_CODE_OK on success, STATUS_CODE_UNINITIALIZED on pxSemaphoreBuffer is NULL
StatusCode sem_init(Semaphore *sem, uint32_t max_count, uint32_t initial_count);

// Obtains previously initiated semaphore and decrements the counting semaphore
// Waits timeout_ms milliseconds for the semaphore to become available
// Returns STATUS_CODE_OK on success, STATUS_CODE_TIMEOUT if xTicksToWait expires
StatusCode sem_wait(Semaphore *sem, uint32_t timeout_ms);

// Releases a semaphore and increments the counting semaphore
// Note that this must not be used from ISR
// An error can occur if there is no space on the queue
// Returns STATUS_CODE_OK if semaphore is successfully released, STATUS_CODE_INTERNAL_ERROR on error
StatusCode sem_post(Semaphore *sem);

// Returns a semaphore's current counting value in case it is a counting semaphore
// If semaphore is binary, returns 1 when the semaphore is available and 0 if it is not
uint32_t sem_num_items(Semaphore *sem);
