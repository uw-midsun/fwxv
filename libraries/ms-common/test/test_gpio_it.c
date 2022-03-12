#include <stdint.h>

#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "queue.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 4 },
  { .port = GPIO_PORT_B, .pin = 3 },
};

static const GpioAddress buttons[] = {
  { .port = GPIO_PORT_A, .pin = 7 },
  { .port = GPIO_PORT_A, .pin = 6 },
};

static InterruptSettings settings = {
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .type = INTERRUPT_TYPE_INTERRUPT,
};

void setup_test(void) {}

void teardown_test(void) {}

static uint32_t triggered = 0;

TASK(handler, TASK_MIN_STACK_SIZE) {
  while (true) {
    triggered = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (int i = 0; i < 2; ++i) {
      if ((triggered & (1u << i)) != 0) {
        gpio_toggle_state(&leds[i]);
      }
    }
  }
}

TASK_TEST(gpio_it_test, TASK_MIN_STACK_SIZE) {
  tasks_init_task(handler, 2, NULL);

  interrupt_init();
  gpio_it_init();

  gpio_it_register_interrupt(&buttons[0], &settings, INTERRUPT_EDGE_RISING_FALLING, handler->handle,
                             0);
  gpio_it_register_interrupt(&buttons[1], &settings, INTERRUPT_EDGE_RISING_FALLING, handler->handle,
                             1);

  for (int i = 0; i < 5; ++i) {
    delay_ms(100);
    gpio_it_trigger_interrupt(&buttons[0]);
    
    TEST_ASSERT_EQUAL(triggered, 0b1);

    triggered = 0;
  }
}
