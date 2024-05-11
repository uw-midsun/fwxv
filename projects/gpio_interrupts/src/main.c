#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
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

GpioAddress buttonAddress = { .port = GPIO_PORT_A, .pin = 7 };

InterruptSettings led_interrupt = { .priority = INTERRUPT_PRIORITY_NORMAL,
                                    .type = INTERRUPT_TYPE_INTERRUPT,
                                    .edge = INTERRUPT_EDGE_RISING };

TASK(GPIO_LED, TASK_STACK_512) {
  gpio_init_pin(&buttonAddress, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  while (true) {
    uint32_t notification = 0;
    notify_wait(&notification, BLOCK_INDEFINITELY);

    if (notification & (1 << BTN_INT_EVENT)) {
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
}

int main() {
  log_init();
  tasks_init();
  interrupt_init();
  gpio_init();

  gpio_it_register_interrupt(&buttonAddress, &led_interrupt, BTN_INT_EVENT, GPIO_LED);

  tasks_init_task(GPIO_LED, TASK_PRIORITY(1), NULL);
  tasks_start();

  return 0;
}
