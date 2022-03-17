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

typedef void (*SoftTimerCallback)(TimerHandle_t handle);

// Adds a software timer. The provided duration is the number of
// miliseconds before running and the callback is the process to run once
// the time has expired.
StatusCode soft_timer_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer *timer);

// Cancels the soft timer specified by name. Returns true if successful.
// the timer is not cancelled immediately,
StatusCode soft_timer_cancel(SoftTimer *timer);

// restart the timer
StatusCode soft_timer_reset(SoftTimer *timer);

// Checks if the software timer is running
bool soft_timer_inuse(SoftTimer *timer);

// Checks the time left in ticks on a particular timer. Returns a 0 if the timer
// has expired and is no longer in use.
uint32_t soft_timer_remaining_time(SoftTimer *timer);
