#pragma once

// Software-based timers using FreeRTOS
// soft timers should only be used for delayed function calls, use tasks and delayUntil for periodic
// code running

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "status.h"
#include "timers.h"

typedef struct SoftTimer {
  StaticTimer_t buffer;
  TimerHandle_t handle;
} SoftTimer;

// Adds a software timer. The provided duration is the number of
// miliseconds before running and the callback is the process to run once
// the time has expired.
#define soft_timer_start(duration_ms, callback, timer)                                           \
  (timer).handle = xTimerCreateStatic(NULL, pdMS_TO_TICKS(duration_ms), pdFALSE, NULL, callback, \
                                      &(timer).buffer);                                          \
  xTimerStart((timer).handle, 0)

// Cancels the soft timer specified by name. Returns true if successful.
#define soft_timer_cancel(timer) xTimerDelete((timer).handle, 0)

// restart the timer
#define soft_timer_reset(timer) xTimerReset((timer).handle, 0)

// Checks if the software timer is running
#define soft_timer_inuse(timer) xTimerIsTimerActive((timer).handle)

// Checks the time left in ticks on a particular timer. Returns a 0 if the timer
// has expired and is no longer in use.
#define soft_timer_remaining_time(timer) \
  soft_timer_inuse(timer) ? xTimerGetExpiryTime((timer).handle) - xTaskGetTickCount() : 0
