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
#include "FreeRTOS.h"
#include "ads1115.h"

TASK(led_task, TASK_STACK_512){

  //Initializing led GPIO address
  const GpioAddress led_addr = {
    .port = GPIO_PORT_A,
    .pin = 15
  };

  //Initializing ADS1115 config and ready pin
  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0
  };
  ADS1115_Config config = {
    .handler_task = led_task,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin
  };

  // Initializing gpio pins
  gpio_init();
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  
  //Initializing I2C
  ads1115_init(&config, ADS1115_ADDR_GND, &ready_pin);
  float reading; 

  while(1){
    gpio_toggle_state(&led_addr);
    delay_s(1);
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &reading);
    delay_ms(100);
  }
}
int main() {
  tasks_init();
  log_init();
  
  
  
  //Initializing task
  tasks_init_task(led_task, TASK_PRIORITY(1), NULL);
  
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
