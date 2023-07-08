#include "delay.h"
#include "log.h"
#include "semaphore.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

static Mutex mut;
static Semaphore sem;
static int test_variable;

DECLARE_TASK(task1);
DECLARE_TASK(task2);
DECLARE_TASK(task3);
DECLARE_TASK(task4);

TASK(task1, TASK_MIN_STACK_SIZE) {
    LOG_DEBUG("task1 started\n");
    mutex_lock(&mut, 10);
    TEST_ASSERT_TRUE(test_variable == 0);
    test_variable = 1;
    TEST_ASSERT_TRUE(test_variable == 1);
    mutex_unlock(&mut);
}

TASK(task2, TASK_MIN_STACK_SIZE) {
    LOG_DEBUG("task2 started\n");
    mutex_lock(&mut, 10);
    TEST_ASSERT_TRUE(test_variable == 1);
    test_variable = 2;
    TEST_ASSERT_TRUE(test_variable == 2);
    mutex_unlock(&mut);
}

// TASK(task3, TASK_MIN_STACK_SIZE) {
//     LOG_DEBUG("task3 started\n");
//     TEST_ASSERT_TRUE(test_variable == 0);
//     test_variable = 3;
//     TEST_ASSERT_TRUE(test_variable == 3);
//     sem_post(&sem);
// }

// TASK(task4, TASK_MIN_STACK_SIZE) {
//     LOG_DEBUG("task4 started\n");
//     TEST_ASSERT_TRUE(test_variable == 0);
//     test_variable = 4;
//     TEST_ASSERT_TRUE(test_variable == 4);
//     sem_post(&sem);
// }

void setup_test(void) {
  log_init();
  tasks_init();
  test_variable = 0;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_mut() {
    mutex_init(&mut);
    tasks_init_task(task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(task2, TASK_PRIORITY(1), NULL);

    delay_ms(1000);
}

TEST_IN_TASK
void test_sem() {
    // sem_init(&sem, 1, 0);

    // sem_wait(&sem);
    // delay_ms(1000);
}
