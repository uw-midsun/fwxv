#include "log.h"
#include "notify.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  log_init();
}
void teardown_test(void) {}

// 32 bits -> 32 possible enumerated events
typedef enum {
  T0 = 0,
  T1,
  T2,
  T3,
  T4,
  T5,
  T6,
  T7,
  T8,
  T9,
  T00,
  T01,
  T02,
  T03,
  T04,
  T05,
  T06,
  T07,
  T08,
  T09,
  T20,
  T21,
  T22,
  T23,
  T24,
  T25,
  T26,
  T27,
  T28,
  T29,
  T30,
  T31,
  NUM_T_EVENTS,
} TestNotifyEvent;

TEST_IN_TASK
void test_invalid_args(void) {
  StatusCode result;
  Event event;
  uint32_t notification;

  // Failure on NULL notification
  event = 0;
  result = event_from_notification(NULL, &event);
  TEST_ASSERT_EQUAL(result, STATUS_CODE_INVALID_ARGS);
  TEST_ASSERT_EQUAL(NUM_T_EVENTS, event);
}

TEST_IN_TASK
void test_empty_notification(void) {
  StatusCode result;
  Event event;
  uint32_t notification;

  // Empty notification
  event = 0;
  notification = 0;
  result = event_from_notification(&notification, &event);
  TEST_ASSERT_OK(result);
  TEST_ASSERT_EQUAL(NUM_T_EVENTS, event);
  TEST_ASSERT_EQUAL(0, notification);
}

TEST_IN_TASK
void test_single_notification(void) {
  StatusCode result;
  Event event;
  uint32_t notification;

  // Notification with first bit flipped
  event = 0;
  notification = 0x00000001;
  result = event_from_notification(&notification, &event);
  TEST_ASSERT_EQUAL(result, STATUS_CODE_INCOMPLETE);
  TEST_ASSERT_EQUAL(T0, event);
  TEST_ASSERT_EQUAL(0, notification);
}

TEST_IN_TASK
void test_all_notifications(void) {
  StatusCode result;
  Event event;
  uint32_t notification;

  // Notification with all bits flipped
  event = INVALID_EVENT;
  notification = 0xFFFFFFFF;

  // Run event from notification 32 times
  // Each parsed event should be 1 less than the previous
  for (TestNotifyEvent out = NUM_T_EVENTS; out > T0; out--) {
    result = event_from_notification(&notification, &event);
    TEST_ASSERT_EQUAL(STATUS_CODE_INCOMPLETE, result);
    TEST_ASSERT_EQUAL(out - 1, event);
  }

  // Parse last notification, should return OK
  result = event_from_notification(&notification, &event);
  TEST_ASSERT_OK(result);
  TEST_ASSERT_EQUAL(0, notification);
}

TEST_IN_TASK
void test_interspersed_notifications(void) {
  StatusCode result;
  Event event;
  uint32_t notification;

  // Form of 1010 1010 for each byte
  notification = 0xAAAAAAAA;
  event = INVALID_EVENT;
  TestNotifyEvent out = T31;

  // This is how projects will parse notifications
  while (event_from_notification(&notification, &event)) {
    TEST_ASSERT_EQUAL(out, event);
    out -= 2;
  }
  TEST_ASSERT_EQUAL(NUM_T_EVENTS, event);
  TEST_ASSERT_EQUAL(0, notification);
}
