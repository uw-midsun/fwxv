#include "delay.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "unity.h"
#include "FreeRTOS.h"
#include "log.h"

bool triggered = false;

void prv_set(TimerHandle_t id) {
  triggered = true;
}

DECLARE_SOFT_TIMER(timer);

TASK_TEST(soft_timer_test, TASK_STACK_1024) {
  triggered = false;
  SoftTimerId id = NULL;
  soft_timer_start(500, prv_set, timer, id);

  delay_ms(499);

  TEST_ASSERT_FALSE(triggered);

  delay_ms(2);

  TEST_ASSERT_TRUE(triggered);
}

TASK_TEST(soft_timer_test_cancel, TASK_STACK_1024) {
  
  LOG_DEBUG("second test\n");
  triggered = false;
  SoftTimerId id = NULL;
  soft_timer_start(500, prv_set, timer, id);

  delay_ms(450);

  soft_timer_cancel(timer);

  TEST_ASSERT_FALSE(triggered);

  delay_ms(50);

  TEST_ASSERT_FALSE(triggered);
}
