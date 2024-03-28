#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"

#define BTN_INT_EVENT 5

GpioAddress addr1 = {
  .port = GPIO_PORT_B,
  .pin = 5,
};

GpioAddress addr2 = {
  .port = GPIO_PORT_B,
  .pin = 4,
};

GpioAddress addr3 = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

GpioAddress addr4 = {
  .port = GPIO_PORT_A,
  .pin = 15,
};

GpioAddress addr5 = {
  .port = GPIO_PORT_A,
  .pin = 7,
};

TASK(GPIO_LED, TASK_STACK_512) {
  LOG_DEBUG("GPIO_LED TASK INITIALIZED\n");
  while (true) {
    // Toggle LED state
    uint32_t notification = 0;
    notify_get(&notification);

    if (notification == BTN_INT_EVENT) {
      LOG_DEBUG("GPIO_LED RUN\n");
      gpio_toggle_state(&addr1);
      gpio_toggle_state(&addr2);
      gpio_toggle_state(&addr3);
      gpio_toggle_state(&addr4);
    }
  }
}

InterruptSettings settings = {
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .type = INTERRUPT_TYPE_INTERRUPT,
  .edge = INTERRUPT_EDGE_FALLING,
};

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Program start...\n");

  interrupt_init();
  gpio_init();
  gpio_it_init();

  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr5, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);

  gpio_it_register_interrupt(&addr5, &settings, BTN_INT_EVENT, GPIO_LED);

  tasks_init_task(GPIO_LED, TASK_PRIORITY(1), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
