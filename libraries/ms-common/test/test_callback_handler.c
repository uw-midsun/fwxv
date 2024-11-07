#include "callback_handler.h"
#include "delay.h"
#include "log.h"
#include "notify.h"
#include "stdbool.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"

void setup_test(void) {
  log_init();
  callback_init(TASK_PRIORITY(1));
}

void teardown_test(void) {}

bool print_num(void *context) {
  LOG_DEBUG("%d has been printed.\n", *(uint8_t *)context);

  // Do not cancel callback immediately after running
  return false;
}

// Register maximum number of callbacks, then trigger all of them successively and unregister.
TEST_IN_TASK
void test_register_max_callbacks() {
  Event registered_events[MAX_CALLBACKS] = { 0 };
  uint8_t test_numbers[MAX_CALLBACKS] = { 0 };

  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    test_numbers[i] = i;
    Event event = register_callback(print_num, test_numbers + i);
    registered_events[i] = event;
    TEST_ASSERT_LESS_OR_EQUAL(MAX_CALLBACKS, event);
  }

  // Test overflow
  Event event = register_callback(print_num, NULL);
  TEST_ASSERT_EQUAL(INVALID_EVENT, event);

  // Try notifying callback task with invalid event
  StatusCode status = notify(callback_task, event);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);

  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    notify(callback_task, registered_events[i]);
    // TEST_ASSERT(result);
  }
  delay_ms(100);
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, cancel_callback(print_num, test_numbers + i));
  }
}

// Register 1 callback, and call it 10 times in succession, then unregister.
TEST_IN_TASK
void test_repeated_callback_trigger() {
  uint32_t num = 0;
  Event event = register_callback(print_num, &num);

  for (uint8_t i = 0; i < 10; ++i) {
    notify(callback_task, event);
    delay_ms(1);
  }
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, cancel_callback(print_num, &num));
}
