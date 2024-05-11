#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
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
  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  while (true) {
    gpio_toggle_state(&addr1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_toggle_state(&addr2);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_toggle_state(&addr3);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_toggle_state(&addr4);
    vTaskDelay(pdMS_TO_TICKS(100));

  }
}

int main() {
  log_init();
  tasks_init();
  gpio_init();

  return 0;
}
