#pragma once

#include "soft_timer.h"

// declare a watchdog, watchdog is refered to by name
typedef SoftTimer WatchDog;

// start watchdog
#define watchdog_start(watchdog, timeout_ms, callback) \
  soft_timer_start(timeout_ms, callback, watchdog)

// :)
#define watchdog_kick(watchdog) soft_timer_reset(watchdog)

#define watchdog_cancel(watchdog) soft_timer_cancel(watchdog)
