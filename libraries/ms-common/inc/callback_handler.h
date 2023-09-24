#pragma once
// Callback handler library. Can register up to 32 callbacks
//
// USAGE:
//
// Define a callback to register.
//
//    bool example_fn(void *context);
//
// If the callback returns true, it will unregister immediately after being called.
// Otherwise, it will stay registered.
//
// Register the callback with any arguments passed as a context pointer.
//
//    uint8_t my_number = 4;
//    uint8_t *arg = &my_number;
//    Event event = register_callback(example_fn, arg)
//
// Trigger the callback by notifying the callback handler task |callback_task|.
// Pass the event returned by |register_callback| in the notification.
//    #include "notify.h"
//    ...
//      notify(callback_task, event);
//
// To manually unregister a callback, use the cancel_callback function.

#include "notify.h"

// Each callback function must have the following signature.
// Return value indicates whether or not the callback
//    should be unregistered immediately after invocation.
typedef bool (*CallbackFn)(void *);

DECLARE_TASK(callback_task);

#define MAX_CALLBACKS 32

// Initialize callback handler.
void callback_init(TaskPriority priority);

// Register a callback function |cb| with generic argument |context|.
// Returns an Event - number between 0 and 31.
// This Event will be needed in order to trigger the callback.
Event register_callback(CallbackFn cb, void *context);

// Manually unregisters the callback.
// This does not need to be done with a "one-shot" callback
//   - i.e a CallbackFn which returns true will cancel automatically after being invoked.
//
// Pass in same cb and context as when registering the callback.
StatusCode cancel_callback(CallbackFn cb, void *context);
