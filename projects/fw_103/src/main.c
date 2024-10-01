/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and read from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
*/

#include <stdio.h>
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "delay.h"
#include"ads1115.h"

ADS1115_Config config;

TASK(read, TASK_STACK_512){
  float converted_reading;
  while(true){
    StatusCode status = ads1115_read_converted(&config, ADS1115_CHANNEL_0, &converted_reading);
    LOG_DEBUG("%.6f", converted_reading);
    delay_ms(1000);
  }
   
};

TASK(setup, TASK_STACK_512){
  GpioAddress ready_pin = {
  .port = GPIO_PORT_B,
  .pin = GPIO_Pin_0,
};

  config.handler_task = read;
  config.i2c_addr = ADS1115_ADDR_GND;
  config.i2c_port = ADS1115_I2C_PORT;
  config.ready_pin = &ready_pin;
StatusCode status = ads1115_init(&config, config.i2c_addr, &ready_pin);
}





int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_init_task(setup, TASK_PRIORITY(1), NULL);
  tasks_init_task(read, TASK_PRIORITY(2), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
