#include <stdbool.h>
#include <unistd.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "semaphore.h"
#include "status.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"

static GpioAddress s_gpio_addr = { .port = GPIO_PORT_A, .pin = 5 };

// Gpio operations from different tasks should be
// protected by a mutex
static Mutex s_gpio_mutex;

void setup_test(void) {
  log_init();
  gpio_init();
  gpio_init_pin(&s_gpio_addr, GPIO_OUTPUT_OPEN_DRAIN, GPIO_STATE_LOW);
}

void teardown_test(void) {}

// Create two tasks here
TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("task1 started\n");
  GpioState state = GPIO_STATE_LOW;
  StatusCode status;

  // Loop forever, because tasks should not exit.
  while (true) {
    mutex_lock(&s_gpio_mutex, 10);
    status = gpio_set_state(&s_gpio_addr, GPIO_STATE_HIGH);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);

    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_HIGH);

    status = gpio_toggle_state(&s_gpio_addr);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);

    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_LOW);
    mutex_unlock(&s_gpio_mutex);
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("task2 started\n");
  GpioState state;
  StatusCode status;

  // Loop forever, because tasks should not exit.
  while (true) {
    mutex_lock(&s_gpio_mutex, 10);
    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_LOW);

    // Task 1 interrupt
    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_LOW);

    status = gpio_toggle_state(&s_gpio_addr);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_HIGH);

    // Task 1 interrupt
    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_LOW);
    mutex_unlock(&s_gpio_mutex);
  }
}

TEST_IN_TASK
void test_running_task(void) {
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  mutex_init(&s_gpio_mutex);
  tasks_init_task(task1, TASK_PRIORITY(2), NULL);  // higher priority
  tasks_init_task(task2, TASK_PRIORITY(1), NULL);

  // To let it run, use a delay.
  delay_ms(1000);
}
