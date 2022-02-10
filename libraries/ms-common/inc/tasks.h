#pragma once

// TODO: document this better.

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "status.h"
#include "task.h"
#include "tasks_impl.h"

// The minimal stack size a task can have.
// If you pass a smaller stack size to TASK(), tasks_init_task() will use this and issue a warning.
#define TASK_MIN_STACK_SIZE configMINIMAL_STACK_SIZE

// Forward declare a task. This should go in a header.
// The task is accessible as a global variable with with the name passed in.
#define DECLARE_TASK(name) _DECLARE_TASK(name)

// Define a task function. This should go in a source file (.c).
// TODO: better docs
#define TASK(name, stack_size) _TASK(name, stack_size)

typedef struct Task {
  TaskHandle_t handle;
  TaskFunction_t task_func;
  char *name;
  StackType_t *stack;
  size_t stack_size;
  StaticTask_t tcb;
  void *context;
} Task;

typedef UBaseType_t TaskPriority;

StatusCode tasks_init_task(Task *task, TaskPriority priority, void *context);

void tasks_start(void);
