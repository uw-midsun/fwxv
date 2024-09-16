/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and read from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
*/

#include <stdio.h>

#include "log.h"
#include "tasks.h"

#include "gpio.h"
#include "delay.h"

// GpioAddress struct
GpioAddress led_addr = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

TASK(led_gpio_task, TASK_STACK_512) {
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  while(true) {
    gpio_toggle_state(&led_addr);
    LOG_DEBUG("Toggling LED\n"); // Added this line to see if something is even happening in the scons build
    delay_ms(1000);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();

  tasks_init_task(led_gpio_task, TASK_PRIORITY(1), NULL);

  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
