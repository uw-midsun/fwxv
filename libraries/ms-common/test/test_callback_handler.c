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
  uint8_t *num = context;
  LOG_DEBUG("%d has been printed.\n", *num);

  // Do not cancel callback immediately after running
  return false;
}

// Register maximum number of callbacks, then trigger all of them successively and unregister.
TASK(register_max_callbacks, TASK_STACK_512) {
  CallbackFn cb = &print_num;

  Event registered_events[MAX_CALLBACKS] = { 0 };
  uint8_t test_numbers[MAX_CALLBACKS] = { 0 };

  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    test_numbers[i] = i;
    Event event = register_callback(cb, test_numbers + i);
    registered_events[i] = event;
    TEST_ASSERT_LESS_OR_EQUAL(MAX_CALLBACKS, event);
  }

  // Wait for callback_overflow test to finish running
  delay_ms(50);

  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    notify(callback_task->handle, registered_events[i]);
    // TEST_ASSERT(result);
  }
  delay_ms(5);
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    TEST_ASSERT_EQUAL(cancel_callback(cb, test_numbers + i), STATUS_CODE_OK);
  }

  while (1) {
  }
}

// Register 1 callback, and call it 10 times in succession, then unregister.
TASK(repeated_callback_trigger, TASK_STACK_512) {
  CallbackFn cb = &print_num;

  uint32_t *num = malloc(sizeof(uint32_t));
  Event event = register_callback(cb, num);

  for (uint8_t i = 0; i < 10; ++i) {
    notify(callback_task->handle, event);
    delay_ms(1);
  }
  TEST_ASSERT_EQUAL(cancel_callback(cb, num), STATUS_CODE_OK);

  free(num);

  while (1) {
  }
}

// Try registering a callback when MAX_CALLBACKS has been reached
// Callback slots get filled from register_max_callbacks task
TASK(callback_overflow, TASK_STACK_512) {
  uint32_t *num = malloc(sizeof(uint32_t));

  Event event = register_callback(print_num, num);
  TEST_ASSERT_EQUAL(event, INVALID_EVENT);

  // Try notifying callback task with invalid event
  StatusCode status = notify(callback_task->handle, event);
  TEST_ASSERT_EQUAL(status, STATUS_CODE_INVALID_ARGS);

  free(num);

  while (1) {
  }
}

TASK_TEST(test_callbacks, TASK_STACK_512) {
  delay_ms(1);
  LOG_DEBUG("test_callbacks\n");
  tasks_init_task(register_max_callbacks, TASK_PRIORITY(1), NULL);
  delay_ms(1);
  tasks_init_task(callback_overflow, TASK_PRIORITY(1), NULL);
  delay_ms(150);
  tasks_init_task(repeated_callback_trigger, TASK_PRIORITY(1), NULL);
  delay_ms(50);
}
