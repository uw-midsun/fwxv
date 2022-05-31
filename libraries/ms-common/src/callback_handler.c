#include "callback_handler.h"

#include "log.h"
#include "tasks.h"
#include "notify.h"
#include "status.h"

typedef struct {
  CallbackFn callback_fn;
  void *context;
} Callback;

static Callback s_callback_storage[MAX_CALLBACKS];

// Bitmap of registered callbacks. A 1-bit represents the callback is registered.
static uint32_t s_registered_callbacks = 0;

void callback_init(TaskPriority priority) {
  StatusCode result = tasks_init_task(callback_task, priority, NULL);
}

// Helper function to find smallest event available to assign to a callback.
Event prv_find_next_event() {
  if (!(~s_registered_callbacks)) {
    // This will happen if all bits in s_registered_callbacks are 1's.
    // I.e. 32 callbacks registered, no more capacity.
    return INVALID_EVENT;
  }
  // Gets index of least significant 0 bit (first available event)
  Event event = __builtin_ctz(~s_registered_callbacks);

  return event;
}

StatusCode prv_trigger_callback(Event event) {
  if (event >= MAX_CALLBACKS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Callback has not been registered
  if (!(s_registered_callbacks & (1u << event))) {
    return STATUS_CODE_INVALID_ARGS;
  }

  // Run callback
  void *context = s_callback_storage[event].context;
  s_callback_storage[event].callback_fn(context);

  // Clear Callback object
  s_callback_storage[event].callback_fn = NULL;
  s_callback_storage[event].context = NULL;

  // Clear bit in bitmask
  s_registered_callbacks &= ~(1u << event);

  return STATUS_CODE_OK;
}

Event register_callback(CallbackFn cb, void *context) {
  Event event = prv_find_next_event();

  if (event >= INVALID_EVENT) {
    LOG_CRITICAL("RESOURCE EXHAUSTED\n");
    return event;
  }

  // Set |event|th bit in s_registered_callbacks to 1.
  s_registered_callbacks |= 1u << event;
  s_callback_storage[event] = (Callback){ .callback_fn = cb, .context = context };

  return event;
}

TASK(callback_task, TASK_STACK_512) {
  uint32_t notification = 0;
  Event event = 0;
  while (true) {
    notify_wait(&notification, BLOCK_INDEFINITELY);
    event_from_notification(&notification, &event);
    prv_trigger_callback(event);
  }
}
