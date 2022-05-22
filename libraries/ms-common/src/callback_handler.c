#include "callback_handler.h"

#include "log.h"
#include "mutex.h"
#include "notify.h"
#include "status.h"

static callback s_callback_storage[MAX_CALLBACKS] = { NULL };
static Event s_first_available_event = 0;

// Bit map of registered callbacks. A 1-bit represents the callback is registered.
static uint32_t s_registered_callbacks = 0;

static Mutex callback_mutex;

void callback_init(void) {
  StatusCode result = tasks_init_task(callback_task, TASK_PRIORITY(tskIDLE_PRIORITY + 1), NULL);
  mutex_init(&callback_mutex);
}

Event prv_find_next_event() {
  // This will happen if all bits in s_registered_callbacks are 1's.
  // I.e. 32 callbacks registered, no more capacity.
  if (~s_registered_callbacks == 0u) {
    return INVALID_EVENT;
  }
  // Gets index of least significant 0 bit (first available event)
  Event event = __builtin_ctz(~s_registered_callbacks);

  return event;
}

StatusCode prv_run_callback(Event event) {
  if (event > 31) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Run callback
  // ADD context field
  s_callback_storage[event]();
  s_callback_storage[event] = NULL;

  // Clear bit in bitmask
  s_registered_callbacks &= ~(1u << event);
  LOG_DEBUG("Ran callback: %d, callbacks is: %08lx\n", event, s_registered_callbacks);

  return STATUS_CODE_OK;
}

Event register_callback(callback cb) {
  mutex_lock(&callback_mutex, BLOCK_INDEFINITELY);
  Event event = prv_find_next_event();

  // Set <event> bit in s_registered_callbacks to 1.
  s_registered_callbacks |= 1u << event;
  s_callback_storage[event] = cb;
  mutex_unlock(&callback_mutex);

  LOG_DEBUG("Registered callback: %d, callbacks is: %08lx\n", event, s_registered_callbacks);
  return event;
}

TASK(callback_task, TASK_STACK_512) {
  uint32_t notification = 0;
  Event event = 0;
  while (true) {
    notify_wait(&notification, BLOCK_INDEFINITELY);
    event_from_notification(&notification, &event);
    prv_run_callback(event);
  }
}
