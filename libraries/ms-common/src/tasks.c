#include "tasks.h"

#include <stdbool.h>

#include "FreeRTOS.h"
#include "log.h"
#include "status.h"

// End task semaphore.
static StaticSemaphore_t s_end_task_sem;
static SemaphoreHandle_t s_end_task_handle;

// Add any setup or teardown that needs to be done for every task here.
static void prv_task(void *params) {
  Task *task = params;
  if (task == NULL) {  // guard just in case, error should have been caught previously
    LOG_CRITICAL("CRITICAL: Tried to start null task!\n");
    return;
  }

  LOG_DEBUG("Task %s starting.\n", task->name);

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
  configASSERT(task->handle != NULL);  // make sure it was created
  return STATUS_CODE_OK;
}

void tasks_start(void) {
  vTaskStartScheduler();

  // We expect the scheduler to stop in task tests, but it's a critical problem otherwise.
#ifndef MS_TEST
  LOG_CRITICAL("CRITICAL: scheduler stopped!\n");
#endif
}

StatusCode tasks_init(void) {
  // Initialize the end task semaphore.
  s_end_task_handle = xSemaphoreCreateCountingStatic(MAX_NUM_TASKS, 0, &s_end_task_sem);

  if (s_end_task_handle == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  } else {
    return STATUS_CODE_OK;
  }
}

StatusCode wait_tasks(uint16_t num_tasks) {
  for (uint16_t i = 0; i < num_tasks; ++i) {
    if (xSemaphoreTake(s_end_task_handle, pdMS_TO_TICKS(WAIT_TASK_TIMEOUT_MS)) != pdTRUE) {
      // Task took longer than 1 second for its execution cycle, return timeout
      return STATUS_CODE_TIMEOUT;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode send_task_end() {
  if (xSemaphoreGive(s_end_task_handle) != pdTRUE) {
    // if giving to semaphore failed, we ran out of space in the buffer
    LOG_CRITICAL("CRITICAL: Out of buffer space in semaphore.");
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  return STATUS_CODE_OK;
}
