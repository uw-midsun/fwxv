#pragma once

#include "soft_timer.h"
#include "status.h"

// declare a watchdog, watchdog is refered to by name
typedef SoftTimer WatchDog;

// start watchdog
#define watchdog_start(name, timeout_ms, callback) soft_timer_start(timeout_ms, callback, name)

// :)
#define watchdog_kick(name) soft_timer_reset(name)

#define watchdog_cancel(name) soft_timer_cancel(name)
