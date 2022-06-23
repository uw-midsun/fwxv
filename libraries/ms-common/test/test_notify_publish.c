#include "delay.h"
#include "log.h"
#include "notify.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

DECLARE_TASK(pub_task);

DECLARE_TASK(sub_task1);
DECLARE_TASK(sub_task2);
DECLARE_TASK(sub_task3);
DECLARE_TASK(sub_task4);
DECLARE_TASK(sub_task5);
DECLARE_TASK(sub_task6);
DECLARE_TASK(sub_task7);

// Publishes to topics
TASK(pub_task, TASK_STACK_512) {
  delay_ms(5);
  TEST_ASSERT_OK(publish(TOPIC_1));
  delay_ms(10);
  TEST_ASSERT_OK(publish(TOPIC_2));
}

TASK(sub_task1, TASK_STACK_512) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task2, TASK_STACK_512) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task3, TASK_STACK_512) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task4, TASK_STACK_512) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task5, TASK_STACK_512) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task6, TASK_STACK_512) {
  uint32_t notification;
  Event e;
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(0, e);
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(1, e);
}

TASK(sub_task7, TASK_STACK_512) {
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

bool inited = false;
void setup_test(void) {
  log_init();

  if (!inited) {
    inited = true;
    // Notify requires initialization before scheduler starts
    tasks_init_task(pub_task, TASK_PRIORITY(1), NULL);

    tasks_init_task(sub_task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(sub_task2, TASK_PRIORITY(1), NULL);
    tasks_init_task(sub_task3, TASK_PRIORITY(1), NULL);
    tasks_init_task(sub_task4, TASK_PRIORITY(1), NULL);
    tasks_init_task(sub_task5, TASK_PRIORITY(1), NULL);
    tasks_init_task(sub_task6, TASK_PRIORITY(1), NULL);
    tasks_init_task(sub_task7, TASK_PRIORITY(1), NULL);

    // Subscribe all tasks to topic 1
    TEST_ASSERT_OK(subscribe(sub_task1->handle, TOPIC_1, 0));
    TEST_ASSERT_OK(subscribe(sub_task2->handle, TOPIC_1, 0));
    TEST_ASSERT_OK(subscribe(sub_task3->handle, TOPIC_1, 0));
    TEST_ASSERT_OK(subscribe(sub_task4->handle, TOPIC_1, 0));
    TEST_ASSERT_OK(subscribe(sub_task5->handle, TOPIC_1, 0));
    TEST_ASSERT_OK(subscribe(sub_task6->handle, TOPIC_1, 0));
    TEST_ASSERT_OK(subscribe(sub_task7->handle, TOPIC_1, 0));

    TEST_ASSERT_OK(subscribe(sub_task1->handle, TOPIC_2, 1));
    TEST_ASSERT_OK(subscribe(sub_task2->handle, TOPIC_2, 1));
    TEST_ASSERT_OK(subscribe(sub_task3->handle, TOPIC_2, 1));
    TEST_ASSERT_OK(subscribe(sub_task4->handle, TOPIC_2, 1));
    TEST_ASSERT_OK(subscribe(sub_task5->handle, TOPIC_2, 1));
    TEST_ASSERT_OK(subscribe(sub_task6->handle, TOPIC_2, 1));
    TEST_ASSERT_OK(subscribe(sub_task7->handle, TOPIC_2, 1));
  }
}

void test_publish() {
  delay_ms(20);
}
