#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "unity.h"

bool triggered = false;

void prv_set(TimerHandle_t id) {
  triggered = true;
}

DECLARE_SOFT_TIMER(timer);

TASK_TEST(soft_timer_test, TASK_STACK_1024) {
  triggered = false;
  soft_timer_start(500, prv_set, timer);

  delay_ms(299);
  TEST_ASSERT_EQUAL(201, soft_timer_remaining_time(timer));
  delay_ms(200);

  TEST_ASSERT_FALSE(triggered);
  TEST_ASSERT_EQUAL(1, soft_timer_remaining_time(timer));

  delay_ms(2);

  TEST_ASSERT_TRUE(triggered);

  delay_ms(100);
}

TASK_TEST(soft_timer_test_cancel, TASK_STACK_1024) {
  triggered = false;
  soft_timer_start(500, prv_set, timer);

  delay_ms(450);

  TEST_ASSERT_TRUE(soft_timer_inuse(timer));

  soft_timer_cancel(timer);

  delay_ms(1);
  TEST_ASSERT_FALSE(triggered);
  TEST_ASSERT_FALSE(soft_timer_inuse(timer));

  delay_ms(50);
  TEST_ASSERT_FALSE(triggered);
  TEST_ASSERT_FALSE(soft_timer_inuse(timer));
}
