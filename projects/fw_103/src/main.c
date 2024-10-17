/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and read from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
*/

#include <stdio.h>

#include "ads1115.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"

GpioAddress led_addr = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

TASK(task1, TASK_STACK_512) {
  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };

  ADS1115_Config config = {
    .handler_task = task1,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };

  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  float reading;

  while (true) {
    gpio_toggle_state(&led_addr);
    ads1115_read_converted(&config, ADS1115_CHANNEL_1, &reading);
    LOG_DEBUG("reading: %f\n", reading);
    delay_s(1);
  }
};

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_init_task(task1, TASK_PRIORITY(1), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
