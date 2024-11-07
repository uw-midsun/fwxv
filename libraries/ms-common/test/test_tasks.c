#include <stdbool.h>

#include "delay.h"
#include "log.h"
#include "semaphore.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

enum test {
  T1,
  MAX,
};

int tests[MAX];

static Semaphore s_sem;

// These are tests for tasks.h, but also an example of using the task_test_helpers.h library.

static bool s_task_started;

void setup_test(void) {
  log_init();
  s_task_started = false;
}

void teardown_test(void) {}

// A simple task to test.
TASK(my_task, TASK_STACK_512) {
  LOG_DEBUG("my_task started\n");
  LOG_DEBUG("Mutex unlocking\n");
  // Loop forever, because tasks should not exit.
  while (1) {
    // Since this task is lower then test task priority
    // It will not run unless test task is waiting on s_sem
    s_task_started = true;
    sem_post(&s_sem);
  }
}

TEST_IN_TASK
void test_running_task() {
  sem_init(&s_sem, 1, 0);
  LOG_DEBUG("started first test\n");
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  tasks_init_task(my_task, TASK_PRIORITY(1), NULL);

  // The task doesn't start immediately because the test task has the highest priority.
  TEST_ASSERT_FALSE(s_task_started);

  // To let it run, use a semaphore
  sem_wait(&s_sem, 1000);

  // The task should have run.
  TEST_ASSERT_TRUE(s_task_started);
}

TEST_IN_TASK
void test_running_task_2() {
  sem_init(&s_sem, 1, 0);
  LOG_DEBUG("started second test\n");
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  tasks_init_task(my_task, TASK_PRIORITY(1), NULL);

  // The task doesn't start immediately because the test task has the highest priority.
  TEST_ASSERT_FALSE(s_task_started);

  // Wait on receiving sem for other task to run
  TEST_ASSERT_OK(sem_wait(&s_sem, 1000));

  // The task should have run.
  TEST_ASSERT_TRUE(s_task_started);
}
