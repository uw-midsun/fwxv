#pragma once
// Callback handler library. Can register up to 32 callbacks
//
// USAGE:
//
// Define a callback to register.
//
//    void example_fn(void *context);
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
//      notify(callback_task->handle, event);

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "tasks.h"

// Each callback function must have the following signature.
typedef void (*CallbackFn)(void *);

DECLARE_TASK(callback_task);

#define MAX_CALLBACKS 32
#define ITEM_SIZE sizeof(CallbackFn)

// Initialize callback handler.
void callback_init(void);

// Register a callback function |cb| with generic argument |context|.
// Returns an Event - number between 0 and 31.
// This Event will be needed in order to trigger the callback.
Event register_callback(CallbackFn cb, void *context);
