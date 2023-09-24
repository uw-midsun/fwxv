#pragma once

#include "soft_timer.h"

// declare a watchdog
typedef SoftTimer Watchdog;
// example: `void callback_fn(WatchdogId id) { ...body }`
typedef SoftTimerCallback WatchdogCallback;
// id used in watchdog callback
typedef SoftTimerId WatchdogId;

/**
 * @brief start a watchdog.
 *
 * @param watchdog Watchdog.
 * @param timeout_ms uint32_t, time before watchdog expires.
 * @param callback WatchdogCallback, called when watchdog expires.
 *
 * @return StatusCode: STATUS_CODE_OK if watchdog is successfully created, otherwise
 * STATUS_CODE_INTERNAL_ERROR.
 */
#define watchdog_start(watchdog, timeout_ms, callback) \
  soft_timer_start(timeout_ms, callback, watchdog)

/**
 * @brief reset the timer on a watchdog.
 *
 * @param watchdog Watchdog.
 *
 * @return StatusCode: STATUS_CODE_OK if watchdog timer is reset successfully, otherwise
 * STATUS_CODE_INTERNAL_ERROR.
 */
#define watchdog_kick(watchdog) soft_timer_reset(watchdog)

/**
 * @brief cancel the watchdog.
 *
 * @param watchdog Watchdog.
 *
 * @return StatusCode: STATUS_CODE_OK if watchdog timer is cancelled successfully, otherwise
 * STATUS_CODE_INTERNAL_ERROR.
 */
#define watchdog_cancel(watchdog) soft_timer_cancel(watchdog)
