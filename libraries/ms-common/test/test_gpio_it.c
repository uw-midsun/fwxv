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
  .edge = INTERRUPT_EDGE_FALLING,
};

static InterruptSettings rising = {
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .type = INTERRUPT_TYPE_INTERRUPT,
  .edge = INTERRUPT_EDGE_RISING,
};

enum {
  BUTTON_0 = 0,
  BUTTON_1,
};

void setup_test(void) {}

void teardown_test(void) {}

static uint32_t triggered = 0;

TASK(handler, TASK_MIN_STACK_SIZE) {
  while (true) {
    notify_get(&triggered);

    for (int i = 0; i < 2; ++i) {
      if ((triggered & (1u << i)) != 0) {
        LOG_DEBUG("triggered pin %i\n", buttons[i].pin);
        gpio_toggle_state(&leds[i]);
      }
    }
  }
}

TEST_IN_TASK
void test_gpio_it(void) {
  tasks_init_task(handler, 2, NULL);

  interrupt_init();
  gpio_it_init();

  gpio_it_register_interrupt(&buttons[0], &settings, BUTTON_0, handler);
  gpio_it_register_interrupt(&buttons[1], &settings, BUTTON_1, handler);

  for (int i = 0; i < 5; ++i) {
    delay_ms(100);
    gpio_it_trigger_interrupt(&buttons[0]);
    delay_ms(100);

    triggered = 0;
  }
}

// test for hardware, uncomment and test with tutorial boards. leds should toggle with the two
// buttons

// void hardware_test() {
//   tasks_init_task(handler, 2, NULL);

//   interrupt_init();
//   gpio_it_init();

//   gpio_it_register_interrupt(&buttons[0], &settings, BUTTON_0, handler);
//   gpio_it_register_interrupt(&buttons[1], &settings, BUTTON_1, handler);

//   while (true) {
//     vTaskDelay(portMAX_DELAY);
//   }
// }
