#include "tasks.h"

#include <stdbool.h>

#include "FreeRTOS.h"
#include "log.h"
#include "status.h"

// Add any setup or teardown that needs to be done for every task here.
static void prv_task(void *params) {
  Task *task = params;

  // Call the task function. This shouldn't exit.
  task->task_func(task->context);

  // If the task somehow exits, warn and clean up properly.
  LOG_WARN("WARNING: Task %s exited.\n", task->name);
  vTaskDelete(NULL);
}

StatusCode tasks_init_task(Task *task, TaskPriority priority, void *context) {
  // Defensively guard against invalid initialization - it's bad to get this wrong.
  if (task == NULL || task->task_func == NULL) {
    LOG_CRITICAL("CRITICAL: Tried to create null task!\n");
    return STATUS_CODE_INVALID_ARGS;
  }

  // Priorities range from 0 to configMAX_PRIORITIES-1 with a higher number meaning higher priority.
  if (priority >= configMAX_PRIORITIES) {
    LOG_CRITICAL("CRITICAL: task '%s' priority is too high, not creating! Was %d, max is %d\n",
                 task->name, (int)priority, configMAX_PRIORITIES - 1);
    return STATUS_CODE_INVALID_ARGS;
  }

  if (task->stack_size < TASK_MIN_STACK_SIZE) {
    LOG_WARN("Task '%s' had too small stack size, defaulting to minimum %d\n", task->name,
             TASK_MIN_STACK_SIZE);
    task->stack_size = TASK_MIN_STACK_SIZE;
  }

  task->context = context;
  task->handle = xTaskCreateStatic(prv_task, task->name, task->stack_size, task, priority,
                                   task->stack, &task->tcb);
  return STATUS_CODE_OK;
}

void tasks_start(void) {
  vTaskStartScheduler();
}
