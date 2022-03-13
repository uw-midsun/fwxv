#pragma once

#include "soft_timer.h"
#include "status.h"

// declare a watchdog, watchdog is refered to by name
#define DECLARE_WATCH_DOG(name) DECLARE_SOFT_TIMER(_watchdog_##name)

// start watchdog
#define watchdog_start(name, timeout_ms, callback) \
  soft_timer_start(timeout_ms, callback, _watchdog_##name)

// :)
#define watchdog_kick(name) soft_timer_reset(_watchdog_##name)

#define watchdog_cancel(name) soft_timer_cancel(_watchdog_##name)
