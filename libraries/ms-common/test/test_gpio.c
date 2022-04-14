#include <stdbool.h>
#include <unistd.h>

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
  GpioState state;
  StatusCode status;

  // Loop forever, because tasks should not exit.
  while (true) {
    LOG_DEBUG("task1 loop start\n");
    status = gpio_set_state(&s_gpio_addr, GPIO_STATE_HIGH);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    usleep(10000);

    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_HIGH);
    usleep(1000);

    status = gpio_toggle_state(&s_gpio_addr);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    usleep(10000);

    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_LOW);

    LOG_DEBUG("task1 loop finished\n");
    delay_ms(10);
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("task2 started\n");
  GpioState state;
  StatusCode status;

  // Loop forever, because tasks should not exit.
  while (true) {  
    LOG_DEBUG("task2 loop start\n");
    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_LOW);
    usleep(10000);

    status = gpio_toggle_state(&s_gpio_addr);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    usleep(10000);

    status = gpio_get_state(&s_gpio_addr, &state);
    TEST_ASSERT_TRUE(status == STATUS_CODE_OK);
    TEST_ASSERT_TRUE(state == GPIO_STATE_HIGH);
    usleep(10000);

    LOG_DEBUG("task2 loop finished\n");
    delay_ms(10);
  }
}

TASK_TEST(test_running_task, TASK_STACK_512) {
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  tasks_init_task(task1, TASK_PRIORITY(2), NULL); // higher priority
  tasks_init_task(task2, TASK_PRIORITY(1), NULL);

  // To let it run, use a delay.
  delay_ms(5000);
}
