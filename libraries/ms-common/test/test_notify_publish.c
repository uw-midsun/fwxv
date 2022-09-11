#include "delay.h"
#include "log.h"
#include "notify.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

DECLARE_TASK(sub_task1);
DECLARE_TASK(sub_task2);
DECLARE_TASK(sub_task3);
DECLARE_TASK(sub_task4);
DECLARE_TASK(sub_task5);
DECLARE_TASK(sub_task6);
DECLARE_TASK(sub_task7);

TASK(sub_task1, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task2, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task3, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task4, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task5, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task6, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task7, TASK_MIN_STACK_SIZE) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

void teardown_test(void) {}

void setup_test(void) {
  log_init();
}

void test_publish_setup() {
  tasks_init_task(sub_task1, TASK_PRIORITY(1), NULL);
  tasks_init_task(sub_task2, TASK_PRIORITY(1), NULL);
  tasks_init_task(sub_task3, TASK_PRIORITY(1), NULL);
  tasks_init_task(sub_task4, TASK_PRIORITY(1), NULL);
  tasks_init_task(sub_task5, TASK_PRIORITY(1), NULL);
  tasks_init_task(sub_task6, TASK_PRIORITY(1), NULL);
  tasks_init_task(sub_task7, TASK_PRIORITY(1), NULL);
  // Subscribe all tasks to topic 1
  TEST_ASSERT_OK(subscribe(sub_task1, TOPIC_1, 0));
  TEST_ASSERT_OK(subscribe(sub_task2, TOPIC_1, 0));
  TEST_ASSERT_OK(subscribe(sub_task3, TOPIC_1, 0));
  TEST_ASSERT_OK(subscribe(sub_task4, TOPIC_1, 0));
  TEST_ASSERT_OK(subscribe(sub_task5, TOPIC_1, 0));
  TEST_ASSERT_OK(subscribe(sub_task6, TOPIC_1, 0));
  TEST_ASSERT_OK(subscribe(sub_task7, TOPIC_1, 0));

  TEST_ASSERT_OK(subscribe(sub_task1, TOPIC_2, 1));
  TEST_ASSERT_OK(subscribe(sub_task2, TOPIC_2, 1));
  TEST_ASSERT_OK(subscribe(sub_task3, TOPIC_2, 1));
  TEST_ASSERT_OK(subscribe(sub_task4, TOPIC_2, 1));
  TEST_ASSERT_OK(subscribe(sub_task5, TOPIC_2, 1));
  TEST_ASSERT_OK(subscribe(sub_task6, TOPIC_2, 1));
  TEST_ASSERT_OK(subscribe(sub_task7, TOPIC_2, 1));
}

TEST_IN_TASK
void test_publish() {
  delay_ms(5);
  TEST_ASSERT_OK(publish(TOPIC_1));
  delay_ms(10);
  TEST_ASSERT_OK(publish(TOPIC_2));
}
