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

#include "ads1115.h"

#include "delay.h"

TASK(ads1115, TASK_STACK_512) {
  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };
  ADS1115_Config config = {
    .handler_task = ads1115,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };

  while (true) {
    float voltage;
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &voltage);
    LOG_DEBUG("%f\n", voltage);
    delay_ms(500);
  }
}

TASK(led_blink, TASK_STACK_512) {
  GpioAddress led_addr = {
    .port = GPIO_PORT_B,
    .pin = 3,
  };

  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  while (true) {
    gpio_toggle_state(&led_addr);
    LOG_DEBUG("blink\n");
    delay_ms(1000);
  }
}

int main() {
  gpio_init();
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(ads1115, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}