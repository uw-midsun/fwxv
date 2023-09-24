#include "delay.h"
#include "log.h"
#include "task_test_helpers.h"
#include "unity.h"
#include "watchdog.h"

static Watchdog s_watchdog;

bool callback_called = false;

static void prv_callback(SoftTimerId id) {
  callback_called = true;
}

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

TEST_IN_TASK
void test_watchdog() {
  callback_called = false;
  watchdog_start(&s_watchdog, 100, prv_callback);

  for (int i = 0; i < 3; ++i) {
    delay_ms(99);
    TEST_ASSERT_FALSE(callback_called);
    watchdog_kick(&s_watchdog);
  }

  delay_ms(10);
  TEST_ASSERT_FALSE(callback_called);
  watchdog_kick(&s_watchdog);

  delay_ms(101);  // timesout watchdog
  TEST_ASSERT_TRUE(callback_called);
}
