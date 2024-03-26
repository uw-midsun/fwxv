#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"

TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 starting\n\n");
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

  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  while (true) {
    gpio_toggle_state(&addr1);
    LOG_DEBUG("LED 1 Toggled\n");
    delay_ms(100);
    gpio_toggle_state(&addr2);
    LOG_DEBUG("LED 2 Toggled\n");
    delay_ms(100);
    gpio_toggle_state(&addr3);
    LOG_DEBUG("LED 3 Toggled\n");
    delay_ms(100);
    gpio_toggle_state(&addr4);
    LOG_DEBUG("LED 4 Toggled\n");
    delay_ms(100);
  }
}

int main(void) {
  gpio_init();
  log_init();
  tasks_init();

  // Create tasks here
  tasks_init_task(task1, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");

  // Start the scheduler
  tasks_start();

  return 0;
}
