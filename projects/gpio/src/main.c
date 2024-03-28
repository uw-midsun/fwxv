#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"

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

TASK(GPIO_LED, TASK_STACK_512) {
  LOG_DEBUG("GPIO_LED TASK INITIALIZED");
  while (true) {
    // Toggle LED state
    gpio_toggle_state(&addr1);
    gpio_toggle_state(&addr2);
    gpio_toggle_state(&addr3);
    gpio_toggle_state(&addr4);
    delay_ms(1000);
  }
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Program start...\n");

  StatusCode gpio_init(void);
  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  tasks_init_task(GPIO_LED, TASK_PRIORITY(1), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
