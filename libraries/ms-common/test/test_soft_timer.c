#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "unity.h"

static bool triggered = false;
static TimerHandle_t last_triggered_id;

void prv_set(TimerHandle_t id) {
  triggered = true;
  last_triggered_id = id;
}

static SoftTimer s_timer;
static SoftTimer s_timer_2;

TASK_TEST(soft_timer_test, TASK_STACK_1024) {
  for (int i = 0; i < 3; ++i) {
    triggered = false;
    // test soft timer start and cancel
    soft_timer_start(50, prv_set, s_timer);

    delay_ms(49);

    TEST_ASSERT_TRUE(soft_timer_inuse(s_timer));

    soft_timer_cancel(s_timer);

    delay_ms(2);  // if soft_timer wasn't cancelled, it should be finished by now, check if
                  // triggered was set
                  // this delay also let soft_timer cancel to be processed
    TEST_ASSERT_FALSE(triggered);
    TEST_ASSERT_FALSE(soft_timer_inuse(s_timer));

    // test start to finish
    soft_timer_start(200, prv_set, s_timer);

    delay_ms(99);

    TEST_ASSERT_EQUAL(101, soft_timer_remaining_time(s_timer));

    delay_ms(99);

    TEST_ASSERT_FALSE(triggered);  // just before soft timer finish, tests timing
    TEST_ASSERT_EQUAL(2, soft_timer_remaining_time(s_timer));

    delay_ms(3);  // soft timer should be finished

    TEST_ASSERT_TRUE(triggered);
  }
}

TASK_TEST(soft_timer_pointer, TASK_STACK_1024) {
  SoftTimer *timer_p = &s_timer;

  triggered = false;
  soft_timer_start(50, prv_set, *timer_p);

  delay_ms(49);

  TEST_ASSERT_FALSE(triggered);
  TEST_ASSERT_EQUAL(1, soft_timer_remaining_time(s_timer));
  TEST_ASSERT_TRUE(soft_timer_inuse(*timer_p));

  delay_ms(2);

  TEST_ASSERT_TRUE(triggered);

  TEST_ASSERT_FALSE(soft_timer_inuse(*timer_p));
}

TASK_TEST(multiple_timer, TASK_STACK_1024) {
  for (int i = 0; i < 3; ++i) {
    triggered = false;
    // test soft timer start and cancel
    soft_timer_start(100, prv_set, s_timer);
    soft_timer_start(50, prv_set, s_timer_2);

    delay_ms(51);

    TEST_ASSERT_TRUE(triggered);
    triggered = false;
    TEST_ASSERT_EQUAL(last_triggered_id, s_timer_2.handle);
    TEST_ASSERT_TRUE(soft_timer_inuse(s_timer));
    TEST_ASSERT_FALSE(soft_timer_inuse(s_timer_2));

    delay_ms(100);

    TEST_ASSERT_TRUE(triggered);
    triggered = false;
    TEST_ASSERT_EQUAL(last_triggered_id, s_timer.handle);
    TEST_ASSERT_FALSE(soft_timer_inuse(s_timer));
    TEST_ASSERT_FALSE(soft_timer_inuse(s_timer_2));
  }
}
