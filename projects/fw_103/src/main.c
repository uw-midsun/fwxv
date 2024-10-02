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
#include "gpio.h"
#include "ads1115.h"


TASK(leds, TASK_STACK_512){
  LOG_DEBUG("task initialized");
  
  //blue led
  GpioAddress led_addr = {
    .port = GPIO_PORT_A,
    .pin = 15,
  };

  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };

  ADS1115_Config config = {
    .handler_task = <your_task_name>,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };

  ADS1115_Channel channel = {
    .ADS1115_CHANNEL_0 = 0
  };

  float reading=0;

  while(1){
    gpio_toggle_state($led_addr);
    
    ads1115_read_converted( &config, channel, &reading);
    delay_ms(1000);
   
  }
	
}


int main() {
  tasks_init();
  log_init();
  gpio_init();

  LOG_DEBUG("Welcome to FW 103!\n");
  
  tasks_init_task(leds, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
