#pragma once

// FreeRTOS task wrapper library.
//
// Usage: Forward declare tasks with DECLARE_TASK() in headers.
//
//   DECLARE_TASK(my_cool_task);
//
// Define task functions with TASK() in source files.
//
//   TASK(my_cool_task, TASK_STACK_512) {
//     // Do any setup.
//     // |context| from tasks_init_task is passed as a void *context parameter.
//     my_module_init(context);
//
//     // All tasks MUST loop forever and cannot return.
//     while (true) {
//       // Wait for events, do some processing, ...
//     }
//   }
//
// In main, initialize tasks with tasks_init_task, then start the scheduler with tasks_start.
// Note: higher priority number means higher priority.
//
//   tasks_init_task(&my_cool_task, TASK_PRIORITY(1), my_context);
//   tasks_start(); // Does not return!

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "status.h"
#include "task.h"
#include "tasks_impl.h"

// Forward declare a task. This should go in a header.
// The task is accessible as a global variable of type Task with the name passed in.
#define DECLARE_TASK(name) _DECLARE_TASK(name)

// Define a task function. This should go in a source file (.c).
// |name| is the name of your task, which should match any previous DECLARE_TASK declarations.
// |stack_size| is the depth of your task's stack - use your judgement to choose.
// The generated function has the following signature:
//   void _prv_your_task_function(void *context)
// where |context| is the context pointer passed to tasks_init_task.
#define TASK(name, stack_size) _TASK(name, stack_size)

// The minimal stack size a task can have.
// If you pass a smaller stack size to TASK(), tasks_init_task() will use this and issue a warning.
#define TASK_MIN_STACK_SIZE configMINIMAL_STACK_SIZE

// Some common stack sizes to make specifying stack sizes more readable.
// If your task is failing for strange reasons, try bumping the stack size one size up.
#define TASK_STACK_256 ((size_t)256)
#define TASK_STACK_512 ((size_t)512)
#define TASK_STACK_1024 ((size_t)1024)
#define TASK_STACK_2048 ((size_t)2048)

// A convenience macro for making declaring priorities more readable.
#define TASK_PRIORITY(prio) ((TaskPriority)prio)

typedef UBaseType_t TaskPriority;

// Represents everything we need to know to initialize a task.
// User code should not rely on the contents of this struct!
typedef struct Task {
  TaskHandle_t handle;
  TaskFunction_t task_func;
  char *name;
  StackType_t *stack;
  size_t stack_size;
  StaticTask_t tcb;
  void *context;
} Task;

// Initialize a task that was previously declared with DECLARE_TASK() or TASK().
// |priority| is the task priority: higher number is higher priority, and the maximum
// is configNUM_TASK_PRIORITIES - 1.
StatusCode tasks_init_task(Task *task, TaskPriority priority, void *context);

// Start the FreeRTOS scheduler to run the tasks that were previously initialized.
// This function should not return!
void tasks_start(void);
