#include <gpio.h>
#include <stdio.h>

#include "delay.h"
#include "log.h"
#include "tasks.h"

// GpioAddress struct
GpioAddress addr1 = {
  .port = GPIO_PORT_B,
  .pin = 5,
};

// GpioAddress struct
GpioAddress addr2 = {
  .port = GPIO_PORT_B,
  .pin = 4,
};

// GpioAddress struct
GpioAddress addr3 = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

// GpioAddress struct
GpioAddress addr4 = {
  .port = GPIO_PORT_A,
  .pin = 15,
};

TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  while (true) {
    LOG_DEBUG("hello\n");
    gpio_toggle_state(&addr1);
    delay_ms(1000);
    gpio_toggle_state(&addr2);
    delay_ms(1000);
    gpio_toggle_state(&addr3);
    delay_ms(1000);
    gpio_toggle_state(&addr4);
    delay_ms(1000);
  }
}

int main() {
  gpio_init();
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(task1, TASK_PRIORITY(1), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
