#pragma once

#include <stddef.h>

#include "FreeRTOS.h"
#include "tasks.h"

// We have an extra highest priority in test mode which we use for the test task.
// This way, test tasks run at a higher priority than every other task.
#define TEST_TASK_PRIORITY (configMAX_PRIORITIES - 1)

#define _TEST_TASK_IMPL(test_name)                        \
  void test_##test_name(void) {                           \
    tasks_init_task(test_name, TEST_TASK_PRIORITY, NULL); \
    tasks_start();                                        \
  }

#define TASK_TEST(test_name, stack_size)                                                  \
  DECLARE_TASK(test_name);                                                                \
  _TEST_TASK_IMPL(test_name);                                                             \
  static void _prv_task_test_##test_name(void);                                           \
  TASK(test_name, stack_size) {                                                           \
    _prv_task_test_##test_name();                                                         \
    vTaskEndScheduler(); /* Automagically stop FreeRTOS when the task function returns */ \
  }                                                                                       \
  static void _prv_task_test_##test_name(void)
