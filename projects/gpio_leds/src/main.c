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

InterruptSettings settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                               .priority = INTERRUPT_PRIORITY_NORMAL,
                               .edge = INTERRUPT_EDGE_RISING };
TASK(GPIO_LED, TASK_STACK_512) {
  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr5, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);

  while (true) {
    uint32_t notification;

    notify_get(&notification);
    if (notify_check_event(&notification, BTN_INT_EVENT)) {
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
void run_slow_cycle() {
  gpio_it_trigger_interrupt(&addr5);
}
int main(void) {
  interrupt_init();

  gpio_it_register_interrupt(&addr5, &settings, BTN_INT_EVENT, GPIO_LED);
  gpio_init();
  tasks_init();
  log_init();

  // tasks_init_task(leds_task, TASK_PRIORITY(2), NULL);

  tasks_start();
  LOG_DEBUG("Blinking LEDs...\n");

  return 0;
}
