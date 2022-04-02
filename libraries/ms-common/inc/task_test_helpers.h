#pragma once

// A helper library for writing tests that run inside a dedicated FreeRTOS task.
//
// Task tests run at a higher priority than every other task. They are useful for integration tests
// that test an entire task or interactions between multiple tasks. If your test only tests logic
// within a task and doesn't need any FreeRTOS features, just write a normal unit test instead.
//
// From within a task test, you can start up other tasks:
//
//   #include "tasks.h"
//   ...
//   tasks_init_task(my_task, TASK_PRIORITY(1), NULL);
//
// Note that since the FreeRTOS scheduler is already running within a test task, there is no need to
// call tasks_start().
//
// Tasks you start up will be at a lower priority than your task, so to let them run, use a delay:
//
//   #include "delay.h"
//   ...
//   delay_ms(10); // pause and run a lower-priority task for 10ms
//
// You can also wait on other FreeRTOS primitives, like queues and event groups. If there's no
// helper method for the kind of event you want to wait on, create one!
//
// See test_tasks.c for an example of using this library.

#include <stddef.h>

#include "FreeRTOS.h"
#include "tasks.h"

// Define a task test with the given name and stack size.
// |stack_size| is the depth of your task's stack - use your judgement to choose a size.
// Usage:
//   TASK_TEST(my_cool_test, TASK_STACK_512) {
//     // run some tests, perform some assertions
//   }
#define TASK_TEST(test_name, stack_size)                                                  \
  DECLARE_TASK(test_name);                                                                \
  _TASK_TEST_IMPL(test_name);                                                             \
  static void _prv_task_test_##test_name(void);                                           \
  TASK(test_name, stack_size) {                                                           \
    _prv_task_test_##test_name();                                                         \
    vTaskEndScheduler(); /* Automagically stop FreeRTOS when the task function returns */ \
  }                                                                                       \
  static void _prv_task_test_##test_name(void)

// We have an extra highest priority in test mode which we use for the test task.
// This way, test tasks run at a higher priority than every other task.
#define TASK_TEST_PRIORITY (configMAX_PRIORITIES - 1)

// Implementation helper for TASK_TEST(), do not call directly.
#define _TASK_TEST_IMPL(test_name)                        \
  void test_##test_name(void) {                           \
    tasks_init_task(test_name, TASK_TEST_PRIORITY, NULL); \
    tasks_start();                                        \
  }
