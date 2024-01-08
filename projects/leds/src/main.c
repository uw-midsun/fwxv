#include <stdbool.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"

Event BTN_INT_EVENT = 5;
GpioAddress addr1 = { .port = GPIO_PORT_B, .pin = 5 };
GpioAddress addr2 = { .port = GPIO_PORT_B, .pin = 4 };
GpioAddress addr3 = { .port = GPIO_PORT_B, .pin = 3 };
GpioAddress addr4 = { .port = GPIO_PORT_A, .pin = 15 };
GpioAddress addr5 = {  // button
  .port = GPIO_PORT_A,
  .pin = 7
};
TASK(GPIO_LED, TASK_STACK_512) {
  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr5, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  while (true) {
    notify_get(GPIO_LED);
    if (notify_check_event(GPIO_LED, BTN_INT_EVENT)) {
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
}

int main(void) {
  gpio_it_register_interrupt(&addr5, INTERRUPT_PRIORITY_NORMAL, BTN_INT_EVENT, GPIO_LED);
  interrupt_init();
  gpio_init();
  tasks_init();
  log_init();

  // tasks_init_task(leds_task, TASK_PRIORITY(2), NULL);

  tasks_start();
  LOG_DEBUG("Blinking LEDs...\n");

  return 0;
}
