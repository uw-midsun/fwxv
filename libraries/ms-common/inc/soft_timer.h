#pragma once
// Software-based timers backed by single hardware timer
// Requires interrupts to be initialized.
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "status.h"
#include "timers.h"

typedef void *SoftTimerId;

TimerCallbackFunction_t 

#define DECLARE_SOFT_TIMER(timer_name)                    \
  static StaticTimer_t _s_timer_buffer_##timer_name = {}; \
  static TimerHandle_t _s_timer_name_##timer_name;

// Adds a software timer. The provided duration is the number of
// microseconds before running and the callback is the process to run once
// the time has expired. The timer_id is set to the id of the timer that
// will run the callback.
#define soft_timer_start(duration_ms, callback, timer_name, soft_timer_id)                  \
  _s_timer_name_##timer_name = xTimerCreateStatic("", duration_ms, pdFALSE, soft_timer_id,  \
                                                  callback, &_s_timer_buffer_##timer_name); \
  xTimerStart(_s_timer_name_##timer_name, 0)

// Cancels the soft timer specified by name. Returns true if successful.
#define soft_timer_cancel(timer_name) xTimerDelete(_s_timer_name_##timer_name)

// Checks if software timers are running. Returns true if any soft timers
// are in use.
#define soft_timer_inuse(timer_name) xTimerIsTimerActive(_s_timer_name_##timer_name) == pdTrues

// Checks the time left on a particular timer. Returns a 0 if the timer
// has expired and is no longer in use, or if timer_id is invalid. Note
// that since timer ids are re-used this could return false values once
// the timer has expired or if it is cancelled.
#define soft_timer_remaining_time(timer_name) \
  xTimerGetExpiryTime(_s_timer_name_##timer_name) - xTaskGetTickCount()
