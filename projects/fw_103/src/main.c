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

// Task to handle our ADC reads
TASK(adc_task, TASK_STACK_256) {
  /* TODO: Make filling this out part of onboarding */
  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };

  /* TODO: Make filling this out part of onboarding */
  ADS1115_Config config = {
    .handler_task = adc_task,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };

  while (true) {
    /* TODO: Make filling this out part of onboarding */
    LOG_DEBUG("Running ADC read\n");
    // Read ADC
    float reading;
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &reading);
    LOG_DEBUG("ADC read %f\n", reading);
    delay_ms(100);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_init_task(adc_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
