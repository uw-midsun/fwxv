/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and read from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
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

ADS1115_Config config = {
  .handler_task = "fw_103",
  .i2c_addr = ADS1115_ADDR_GND,
  .i2c_port = ADS1115_I2C_PORT,
  .ready_pin = &ready_pin,
};

int main() {
  tasks_init();
  log_init();
  gpio_init();
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_start();

  GpioState out_state;
  while (true) {
    // Sets the value of out_state to current state of input pin at my_address
    gpio_get_state(&led_addr, &out_state);
    gpio_toggle_state(&led_addr);
    delay_ms(1000);
    ads1115_read_converted(&config, &config.i2c_addr, 65535);
  };

  LOG_DEBUG("exiting main?\n");
  return 0;
}
