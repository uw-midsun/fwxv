/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and read from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
*/

#include "FreeRTOS.h"
#include <stdio.h>
#include "ads1115.h"
#include "log.h"
#include "tasks.h"
#include "gpio.h"
#include "delay.h"


TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
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

  // Initialize the ADS1115
  ads1115_init(&config, ADS1115_ADDR_GND, &ready_pin);

  float voltage;

  while (true) {
    LOG_DEBUG("Entered While Loop\n");

    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &voltage);

    LOG_DEBUG("Channel 0 Voltage: %.3f V\n", voltage);

    delay_ms(1000);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");


  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  LOG_DEBUG("task init\n");

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
