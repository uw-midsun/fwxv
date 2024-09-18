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

// GpioAddress struct
GpioAddress led_addr = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

TASK(led_gpio_task, TASK_STACK_512) {
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  float reading;

  //Ngl, i'm not fully sure why this is being done in the same task as the led thing from before but this seems to be what the instructions are saying.
  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };
  ADS1115_Config config = {
    .handler_task = led_gpio_task,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };

  while(true) {
    gpio_toggle_state(&led_addr);
    LOG_DEBUG("Toggling LED\n"); // Added this line to see if something is even happening in the scons build
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &reading);
    LOG_DEBUG("Reading: %f\n", reading); // Added to see if a value actually comes
    delay_ms(1000);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();

  tasks_init_task(led_gpio_task, TASK_PRIORITY(1), NULL);

  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
