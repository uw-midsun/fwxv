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
#include "inc/ads1115.h"

TASK(task1, TASK_STACK_512){
  
    GpioAddress addr = {
    .port = GPIO_PORT_A,
    .pin = 15,
  };
  StatusCode ret;
  while(1){
    ret =  gpio_toggle_state(&addr);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

TASK(task2, TASK_STACK_512){
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
  float reading;
  while(1){
/*StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {*/
    ads1115_read_converted(&config, ADS1115_CHANNEL_0,&reading);
    LOG_DEBUG("Voltage: %f\n", reading);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
