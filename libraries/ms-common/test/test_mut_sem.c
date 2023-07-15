#include "delay.h"
#include "log.h"
#include "semaphore.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

static Semaphore sem_task1, sem_task2, sem_task3, sem_task4;
static int test_variable;

DECLARE_TASK(task1);
DECLARE_TASK(task2);
DECLARE_TASK(task3);
DECLARE_TASK(task4);

TASK(task1, TASK_MIN_STACK_SIZE) {
    LOG_DEBUG("task1 started\n");
    test_variable = 1;
    sem_post(&sem_task1);
}

TASK(task2, TASK_MIN_STACK_SIZE) {
    LOG_DEBUG("task2 started\n");
    test_variable = 2;
    sem_post(&sem_task2);
}

TASK(task3, TASK_MIN_STACK_SIZE) {
    LOG_DEBUG("task3 started\n");
    test_variable = 3;
    sem_post(&sem_task3);
}

TASK(task4, TASK_MIN_STACK_SIZE) {
    LOG_DEBUG("task4 started\n");
    test_variable = 4;
    sem_post(&sem_task4);
}

void setup_test(void) {
  log_init();
  tasks_init();
  test_variable = 0;
}

void teardown_test(void) {}

TEST_IN_TASK
void test_mut() {
    sem_init(&sem_task1, 1, 0);
    sem_init(&sem_task2, 1, 0);
    sem_init(&sem_task3, 1, 0);
    sem_init(&sem_task4, 1, 0);

    tasks_init_task(task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(task2, TASK_PRIORITY(1), NULL);
    tasks_init_task(task3, TASK_PRIORITY(1), NULL);
    tasks_init_task(task4, TASK_PRIORITY(1), NULL);

    TEST_ASSERT_TRUE(test_variable == 0);
    sem_wait(&sem_task1, 1000);
    TEST_ASSERT_TRUE(test_variable == 1);

    TEST_ASSERT_TRUE(test_variable == 1);
    sem_wait(&sem_task2, 1000);
    TEST_ASSERT_TRUE(test_variable == 2);

    TEST_ASSERT_TRUE(test_variable == 2);
    sem_wait(&sem_task3, 1000);
    TEST_ASSERT_TRUE(test_variable == 3);

    TEST_ASSERT_TRUE(test_variable == 3);
    sem_wait(&sem_task4, 1000);
    TEST_ASSERT_TRUE(test_variable == 4);

    TEST_ASSERT_TRUE(test_variable == 4);
    sem_wait(&sem_task2, 1000);
    TEST_ASSERT_TRUE(test_variable == 2);

    TEST_ASSERT_TRUE(test_variable == 2);
    sem_wait(&sem_task4, 1000);
    TEST_ASSERT_TRUE(test_variable == 4);

    TEST_ASSERT_TRUE(test_variable == 4);
    sem_wait(&sem_task1, 1000);
    TEST_ASSERT_TRUE(test_variable == 1);

    TEST_ASSERT_TRUE(test_variable == 1);
    sem_wait(&sem_task3, 1000);
    TEST_ASSERT_TRUE(test_variable == 3);
}
