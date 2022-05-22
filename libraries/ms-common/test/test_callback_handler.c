#include "callback_handler.h"
#include "delay.h"
#include "log.h"
#include "notify.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"

void setup_test(void) {
  log_init();
  callback_init();
}

void teardown_test(void) {}

void hello_world(void) {
  LOG_DEBUG("Hello\n");
  delay_ms(50);
  LOG_DEBUG("world\n");
}

TASK(basic_callback, TASK_STACK_512) {
  callback cb = &hello_world;

  Event event = register_callback(cb);

  LOG_DEBUG("Callback %d registered: waiting 300ms\n", event);

  delay_ms(300);

  notify(callback_task->handle, event);
  while (true) {
  }
}

TASK_TEST(test_callbacks, TASK_STACK_512) {
  delay_ms(1);
  LOG_DEBUG("test_callbacks\n");
  tasks_init_task(basic_callback, TASK_PRIORITY(1), NULL);

  delay_ms(1000);
}
