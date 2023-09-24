#pragma once

#include "soft_timer.h"

// declare a watchdog, watchdog is refered to by name
typedef SoftTimer WatchDog;
typedef SoftTimerCallback WatchDogCallback;
typedef SoftTimerId WatchDogId;

/**
 * @brief start a watchdog
 *
 * @param watchdog Watchdog
 * @param timeout_ms uint32_t, time before watchdog expires
 * @param callback WatchDogCallback, called when watchdog expires
 *
 * @return STATUS_CODE_OK if watchdog is successfully created, STATUS_CODE_INTERNAL_ERROR otherwise
 */
#define watchdog_start(watchdog, timeout_ms, callback) \
  soft_timer_start(timeout_ms, callback, watchdog)

/**
 * @brief reset the timer on a watchdog
 *
 * @param watchdog Watchdog
 *
 * @return STATUS_CODE_OK if watchdog timer is reset successfully, STATUS_CODE_INTERNAL_ERROR
 * otherwise
 *
 */
#define watchdog_kick(watchdog) soft_timer_reset(watchdog)

#define watchdog_cancel(watchdog) soft_timer_cancel(watchdog)
