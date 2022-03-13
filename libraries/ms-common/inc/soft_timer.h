#pragma once

// Software-based timers using FreeRTOS
// soft timers should only be used for delayed function calls, use tasks and delayUntil for periodic
// code running

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "status.h"
#include "timers.h"

// declare a soft timer, soft timers are refered to by their name
#define DECLARE_SOFT_TIMER(name)               \
  static StaticTimer_t _s_timer_buffer_##name; \
  static TimerHandle_t _s_timer_##name

// Adds a software timer. The provided duration is the number of
// miliseconds before running and the callback is the process to run once
// the time has expired.
#define soft_timer_start(duration_ms, callback, name)                                             \
  _s_timer_##name = xTimerCreateStatic(NULL, pdMS_TO_TICKS(duration_ms), pdFALSE, NULL, callback, \
                                       &_s_timer_buffer_##name);                                  \
  xTimerStart(_s_timer_##name, 0)

// Cancels the soft timer specified by name. Returns true if successful.
#define soft_timer_cancel(name) xTimerDelete(_s_timer_##name, 0)

// restart the timer
#define soft_timer_reset(name) xTimerReset(_s_timer_##name, 0)

// Checks if the software timer is running
#define soft_timer_inuse(name) xTimerIsTimerActive(_s_timer_##name)

// Checks the time left in ticks on a particular timer. Returns a 0 if the timer
// has expired and is no longer in use.
#define soft_timer_remaining_time(name) \
  soft_timer_inuse(name) ? xTimerGetExpiryTime(_s_timer_##name) - xTaskGetTickCount() : 0
