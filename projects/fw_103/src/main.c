#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include "gpio.h"
#include "delay.h"

// Pin Declaration
GpioAddress led_addr = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

// LED toggler
TASK(run_leds, TASK_STACK_512) {
  GpioState current_state;

  while(true) {
    gpio_toggle_state(&led_addr);
    delay_ms(1000);

    // Testing
    gpio_get_state(&led_addr, &current_state);

    if (current_state == GPIO_STATE_LOW) {
      LOG_DEBUG("led blinked off\n");
    } else if (current_state == GPIO_STATE_HIGH) {
      LOG_DEBUG("led blinked on\n");
    } 
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();

  // Initialize PB3_LED_RED
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  // Create task
  tasks_init_task(run_leds, TASK_PRIORITY(1), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
