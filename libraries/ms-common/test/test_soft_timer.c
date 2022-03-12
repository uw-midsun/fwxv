#include "delay.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"
#include "FreeRTOS.h"

bool triggered = false;

void prv_set(TimerHandle_t id) {
  triggered = true;
}

DECLARE_SOFT_TIMER(timer);

TASK_TEST(soft_timer_test, TASK_STACK_1024) {
  SoftTimerId id;
  soft_timer_start(500, prv_set, timer, id);

  delay_ms(499);

  TEST_ASSERT(triggered == false);

  delay_ms(2);

  TEST_ASSERT(triggered == true);
}
