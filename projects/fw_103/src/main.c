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

 



TASK(LEDS, TASK_STACK_512) {
  LOG_DEBUG("LEDS task initialized!\n");
  GpioAddress LEDYELLOW = { .port = GPIO_PORT_B, .pin = 4 };
   GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };

  ADS1115_Config config = {
    .handler_task = LEDS,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };
  while (true) {
    gpio_toggle_state(&LEDYELLOW);
    delay_ms(1000);
  }
}

int main() {
  gpio_init();
  tasks_init();
  tasks_init_task(LEDS, TASK_PRIORITY(1), NULL);
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
