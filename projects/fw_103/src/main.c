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
#include "delay.h"

TASK(my_task, TASK_STACK_512) {
  // Initialization of any variables before task operation begins
    GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };
  
  ADS1115_Channel channel = 5;

  ADS1115_Config config = {
    .handler_task = LEDS,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };
  
  float reading = 1.0;
  
  while(true) {
    gpio_toggle_state(&led_addr);
    delay_ms(1000);
    ads1115_read_converted(&config, channel, &reading);
    LOG_DEBUG("%f\n", reading);
    
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
}
