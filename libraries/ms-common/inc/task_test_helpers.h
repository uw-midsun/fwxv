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

#include "tasks.h"

DECLARE_TASK(test_task);

#ifndef TEST_PRE_TASK
// define in-task/pre-task tests
#define TEST_PRE_TASK
#define TEST_IN_TASK
#endif
