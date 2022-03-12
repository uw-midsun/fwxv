// #include "watchdog.h"
// #include "timers.h"

// void prv_expiry_callback(SoftTimerId timer_id, void *context) {
//   WatchdogStorage *storage = (WatchdogStorage *)context;
//   storage->callback(storage->callback_context);
// }

// void watchdog_start(WatchdogStorage *storage, WatchdogTimeout timeout_ms,
//                     WatchdogExpiryCallback callback, void *context) {
//   storage->timeout_ms = timeout_ms;
//   storage->callback = callback;
//   storage->callback_context = context;
//   storage->timer_id = SOFT_TIMER_INVALID_TIMER;
//   watchdog_kick(storage);
// }

// void watchdog_kick(WatchdogStorage *storage) {
//   xTimerReset();
// }

// void watchdog_cancel(WatchdogStorage *storage) {
//   xTimerDelete();
// }