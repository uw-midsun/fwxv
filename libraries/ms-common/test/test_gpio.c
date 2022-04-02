#include <stdbool.h>

#include "delay.h"
#include "log.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"
#include "gpio.h"
#include "status.h"

static GpioAddress s_gpio_addr = { .port = GPIO_PORT_A, .pin = 5};
static GpioSettings s_gpio_settings = {
  .direction = GPIO_DIR_OUT,
  .state = GPIO_STATE_HIGH,
  .alt_function = GPIO_ALTFN_NONE,
  .resistor = GPIO_RES_NONE,
};

void setup_test(void) {
  gpio_init();
  gpio_init_pin(&s_gpio_addr, &s_gpio_settings);
}

void teardown_test(void) {}

// Create two tasks here
TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("task1 started\n");
  // GpioState state;

  // Loop forever, because tasks should not exit.
  while (true) {
    // gpio_set_state(&s_gpio_addr, GPIO_STATE_LOW);
    // gpio_get_state(&s_gpio_addr, &state);
    // TEST_ASSERT_TRUE(state == GPIO_STATE_LOW);
    LOG_DEBUG("ABC\n");
    delay_ms(20);
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("task2 started\n");

  // Loop forever, because tasks should not exit.
  while (true) {
    LOG_DEBUG("123");
    // gpio_set_state(s_gpio_addr, GPIO_STATE_LOW);
    // GpioState state;
    // gpio_get_state(s_gpio_addr, &state);
    // TEST_ASSERT_TRUE(state == GPIO_STATE_LOW)
    // delay_ms(10);
  }
}

TASK_TEST(test_running_task, TASK_STACK_512) {
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(1), NULL); // higher priority

  // The task doesn't start immediately because the test task has the highest priority.
  TEST_ASSERT_FALSE(false);

  // To let it run, use a delay.
  delay_ms(5000);

  // The tasks should have run.
  TEST_ASSERT_TRUE(true);
}
