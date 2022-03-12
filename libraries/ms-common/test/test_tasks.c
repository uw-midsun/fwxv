#include "tasks.h"

#include <stdbool.h>

#include "delay.h"
#include "log.h"
#include "task_test_helpers.h"
#include "unity.h"

// These are tests for tasks.h, but also an example of using the task_test_helpers.h library.

static bool s_task_started;

void setup_test(void) {
  s_task_started = false;
}

void teardown_test(void) {}

// A simple task to test.
TASK(my_task, TASK_STACK_512) {
  LOG_DEBUG("my_task started\n");
  s_task_started = true;

  // Loop forever, because tasks should not exit.
  while (true) {
  }
}

TASK_TEST(test_running_task, TASK_STACK_512) {
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  tasks_init_task(my_task, TASK_PRIORITY(1), NULL);

  // The task doesn't start immediately because the test task has the highest priority.
  TEST_ASSERT_FALSE(s_task_started);

  // To let it run, use a delay.
  delay_ms(20);

  // The task should have run.
  TEST_ASSERT_TRUE(s_task_started);
}
