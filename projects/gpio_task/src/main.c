#include <stdio.h>

#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "tasks.h"

#define BTN_INT_EVENT 5

static InterruptSettings settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .edge = INTERRUPT_EDGE_RISING,
};

static GpioAddress led1 = {
  .port = GPIO_PORT_B,
  .pin = 5,
};

static GpioAddress led2 = {
  .port = GPIO_PORT_B,
  .pin = 4,
};

static GpioAddress led3 = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

static GpioAddress led4 = {
  .port = GPIO_PORT_A,
  .pin = 15,
};

static GpioAddress button = {
  .port = GPIO_PORT_A,
  .pin = 7,
};

uint32_t notif;

TASK(gpio_led, TASK_STACK_512) {
  LOG_DEBUG("Initializing GPIO_LED Task!\n");

  while (true) {
    notify_get(&notif);
    if (notify_check_event(&notif, BTN_INT_EVENT)) {
      gpio_toggle_state(&led1);
      gpio_toggle_state(&led2);
      gpio_toggle_state(&led3);
      gpio_toggle_state(&led4);
    }
  }
}

int main(void) {
  log_init();

  interrupt_init();
  gpio_init();
  gpio_it_init();

  gpio_init_pin(&led1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&led2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&led3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&led4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&button, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

  tasks_init();
  tasks_init_task(gpio_led, TASK_PRIORITY(2), NULL);

  gpio_it_register_interrupt(&button, &settings, BTN_INT_EVENT, gpio_led);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
