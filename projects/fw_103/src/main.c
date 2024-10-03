/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and  from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
*/

#include <stdio.h>
#include "gpio.h"

#include "log.h"
#include "tasks.h"
#include "delay.h"
#include "ads1115.h"

TASK(my_task, TASK_STACK_512) {
  GpioAddress led_addr = {
    .port = GPIO_PORT_B,
    .pin = 3,
  };

  ADS1115_Config config = {
    .handler_task = my_task,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
  };

  while(true) {
    float reading = 0;
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &reading);
    gpio_toggle_state(&led_addr);
    delay_ms(1000);
    LOG_DEBUG("Reading:%f\n", reading);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");
  tasks_init_task(my_task, TASK_PRIORITY(2), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
