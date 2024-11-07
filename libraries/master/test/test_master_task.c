#include <stdbool.h>

#include "delay.h"
#include "log.h"
#include "master_task.h"
#include "semphr.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

static SemaphoreHandle_t s_sem_handle;
static StaticSemaphore_t s_sem;
static SemaphoreHandle_t s_sem2_handle;
static StaticSemaphore_t s_sem2;

// These are tests for tasks.h, but also an example of using the task_test_helpers.h library.

static bool s_task_started;

void pre_loop_init() {}

void run_fast_cycle() {
  LOG_DEBUG("run_fast_cycle started\n");
}

void run_medium_cycle() {
  LOG_DEBUG("run_medium_cycle started\n");
  xSemaphoreGive(s_sem_handle);
  wait_tasks(1);
}

void run_slow_cycle() {
  LOG_DEBUG("run_slow_cycle started\n");
  xSemaphoreGive(s_sem2_handle);
}

void setup_test(void) {
  log_init();
  s_task_started = false;
}

void teardown_test(void) {}

// A simple task to test.
TASK(my_task, TASK_STACK_512) {
  LOG_DEBUG("Mutex unlocking\n");
  // Loop forever, because tasks should not exit.
  while (1) {
    // Since this task is lower then test task priority
    // It will not run unless test task is waiting on s_sem
    s_task_started = true;
    xSemaphoreTake(s_sem_handle, portMAX_DELAY);

    delay_ms(101);
    LOG_DEBUG("Done delaying\n");

    send_task_end();
  }
}

TEST_IN_TASK
void test_running_task() {
  s_sem_handle = xSemaphoreCreateBinaryStatic(&s_sem);
  s_sem2_handle = xSemaphoreCreateBinaryStatic(&s_sem2);
  set_medium_cycle_count(2);
  set_slow_cycle_count(3);
  LOG_DEBUG("started first test\n");
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  tasks_init_task(my_task, TASK_PRIORITY(1), NULL);
  init_master_task();

  // The task doesn't start immediately because the test task has the highest priority.
  TEST_ASSERT_FALSE(s_task_started);

  // To let it run, use a semaphore
  xSemaphoreTake(s_sem2_handle, portMAX_DELAY);

  // The task should have run.
  TEST_ASSERT_TRUE(s_task_started);
  LOG_DEBUG("checking \n");
  // Test task picks up right before master_task ends so should be 2
  TEST_ASSERT_EQUAL(1, get_cycles_over());

  // To let it run, use a semaphore
  xSemaphoreTake(s_sem2_handle, portMAX_DELAY);
  LOG_DEBUG("checking \n");
  // Test task picks up right before master_task ends so should be 1
  TEST_ASSERT_EQUAL(0, get_cycles_over());

  // To let it run, use a semaphore
  xSemaphoreTake(s_sem2_handle, portMAX_DELAY);
  LOG_DEBUG("checking \n");
  // Test task picks up right before master_task ends so should be 2
  TEST_ASSERT_EQUAL(1, get_cycles_over());

  // To let it run, use a semaphore
  xSemaphoreTake(s_sem2_handle, portMAX_DELAY);
  LOG_DEBUG("checking \n");
  // Test task picks up right before master_task ends so should be 1
  TEST_ASSERT_EQUAL(0, get_cycles_over());
}
