#include <stdint.h>

#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "queue.h"
#include "tasks.h"
#include "unity.h"

static GpioAddress addrA0 = { .port = GPIO_PORT_A, .pin = 0 };
static GpioAddress addrA1 = { .port = GPIO_PORT_A, .pin = 1 };
static InterruptSettings settings = {
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .type = INTERRUPT_TYPE_INTERRUPT,
};

void setup_test(void) {}

void teardown_test(void) {}

static uint32_t triggered = 0;

TASK(handler, TASK_MIN_STACK_SIZE) {
  while (true) {
    xTaskNotifyWait(0, UINT32_MAX, &triggered, portMAX_DELAY);
  }
}

TASK(controller, TASK_MIN_STACK_SIZE) {
  for (int i = 0; i < 5; ++i) {
    delay_ms(50);
    gpio_it_trigger_interrupt(&addrA0);
    TEST_ASSERT_EQUAL(triggered, 0b1);

    triggered = 0;
  }
  vTaskEndScheduler();
}

void test_gpio_it() {
  tasks_init_task(handler, 2, NULL);
  tasks_init_task(controller, 1, NULL);
  
  interrupt_init();
  gpio_it_init();

  gpio_it_register_interrupt(&addrA0, &settings, INTERRUPT_EDGE_RISING_FALLING, handler->handle, 0);

  vTaskStartScheduler();
}

//
// TEST 2
//

TASK(controller_2, TASK_MIN_STACK_SIZE) {
  for (int i = 0; i < 5; ++i) {
    delay_ms(50);
    gpio_it_trigger_interrupt(&addrA0);
    gpio_it_trigger_interrupt(&addrA1);

    delay_ms(10);

    TEST_ASSERT_EQUAL(triggered, 0b101);

    triggered = 0;
  }
  vTaskEndScheduler();
}

void test_gpio_it_2() {
  tasks_init_task(handler, 2, NULL);
  tasks_init_task(controller, 1, NULL);
  
  interrupt_init();
  gpio_it_init();

  gpio_it_register_interrupt(&addrA0, &settings, INTERRUPT_EDGE_RISING_FALLING, handler->handle, 0);
  gpio_it_register_interrupt(&addrA1, &settings, INTERRUPT_EDGE_RISING_FALLING, handler->handle, 2);

  vTaskStartScheduler();
}
