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
#include "ads1115.h"

GpioAddress led_addr = {
  .port = GPIO_PORT_B,
  .pin = 3,
};


TASK(blink, TASK_STACK_512)
{
        LOG_DEBUG("blink Initialized!\n"); 

  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };
  ADS1115_Config config = {
    .handler_task = blink,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };
        float ret; 
        while (true) {
                gpio_toggle_state(&led_addr);

                ads1115_read_converted(&config, ADS1115_CHANNEL_0, &ret); 

                LOG_DEBUG("%f\n", ret); 

                delay_ms(1000);
        }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  LOG_DEBUG("Welcome to FW 103!\n");
 tasks_init_task(blink, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
