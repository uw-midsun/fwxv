/*
    Project Code for FW 103

    Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

    Requirements:
        - Implement the ADC driver functions (set config, select and read from a channel)
        - ADC task to periodically measure the voltage of channel 0
        - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V -
   1V)
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

GpioAddress ready_pin = {
  .port = GPIO_PORT_B,
  .pin = GPIO_Pin_0,
};

TASK(led_task, TASK_STACK_512) {
  while (true) {
    gpio_toggle_state(&led_addr);
    LOG_DEBUG("Pretended to toggle light\n");
    delay_ms(1000);
  }
}

TASK(ads1115_task, TASK_STACK_512) {
  ADS1115_Config config = {
    .handler_task = ads1115_task,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };

  while (true) {
    uint16_t raw_reading;
    ads1115_read_raw(&config, ADS1115_CHANNEL_0, &raw_reading);

    float converted_reading;
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &converted_reading);

    LOG_DEBUG("Converted Reading: %u\n", (unsigned int)raw_reading);
    LOG_DEBUG("Converted Reading: %f\n", converted_reading);

    delay_ms(500);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();

  LOG_DEBUG("Welcome to FW 103!\n");

  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  tasks_init_task(led_task, TASK_PRIORITY(2), NULL);
  tasks_init_task(ads1115_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
