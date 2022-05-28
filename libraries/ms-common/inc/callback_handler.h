#pragma once
// Callback handler library
// TODO(daniel):
// - add void *context to callback signature
// - add comprehensive tests
// - add documentation

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "notify.h"
#include "queue.h"
#include "task.h"
#include "tasks.h"

typedef void (*CallbackFn)(void *);

DECLARE_TASK(callback_task);

#define MAX_CALLBACKS 32
#define ITEM_SIZE sizeof(CallbackFn)

void callback_init(void);

Event register_callback(CallbackFn, void *context);
