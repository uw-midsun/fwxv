#include "delay.h"
#include "log.h"
#include "notify.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  log_init();
}
void teardown_test(void) {}

#define NUM_TEST_EVENTS 7
static Event s_notify_events[NUM_TEST_EVENTS] = { 0, 1, 2, 4, 8, 16, 31 };

DECLARE_TASK(notify_task);
DECLARE_TASK(receive_task);
// Notifies task under test
TASK(notify_task, TASK_STACK_512) {
  // Delay, then send first message
  delay_ms(1);
  TEST_ASSERT_OK(notify(receive_task->handle, s_notify_events[0]));
  // Allow receive to catch up
  delay_ms(1);
  // Send all above notification in one fell swoop
  for (uint8_t i = 0; i < NUM_TEST_EVENTS; i++) {
    TEST_ASSERT_OK(notify(receive_task->handle, s_notify_events[i]));
  }
}

TASK(receive_task, TASK_STACK_512) {
  uint32_t notification;
  Event e;
  // First notify call, nothing should have arrived
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, notify_get(&notification));
  TEST_ASSERT_EQUAL(0, notification);

  // Second notify receive will receive first sent notification
  TEST_ASSERT_OK(notify_wait(&notification, 10));
  TEST_ASSERT_OK(event_from_notification(&notification, &e));
  TEST_ASSERT_EQUAL(s_notify_events[0], notification);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, notify_get(&notification));

  // Wait for notifications to stack up, then iterate through received
  delay_ms(1);
  uint8_t i = 0;
  while (event_from_notification(&notification, &e)) {
    TEST_ASSERT_EQUAL(s_notify_events[i], e);
    i++;
  }
}

TEST_IN_TASK
void test_invalid_args(void) {
  StatusCode result;
  result = notify_get(NULL);
  TEST_ASSERT_EQUAL(result, STATUS_CODE_INVALID_ARGS);
}

TEST_IN_TASK
void test_notifications() {
  tasks_init_task(notify_task, TASK_PRIORITY(1), NULL);
  tasks_init_task(receive_task, TASK_PRIORITY(1), NULL);

  delay_ms(20);
}
