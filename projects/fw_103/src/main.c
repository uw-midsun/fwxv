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

// Define ADC task
TASK(run_leds, TASK_STACK_512) {
  // Define LED pin
  GpioAddress led_addr = {
    // PB3_LED_RED
    .port = GPIO_PORT_B,
    .pin = 3,
  };

  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };

  ADS1115_Config config = {
    .handler_task = run_leds,
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };

  // Track current LED pin state
  GpioState current_led_state;
  // ADC reading buffer
  float buff;

  // Initialize LED pin
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  ads1115_init(&config, ADS1115_ADDR_GND, &ready_pin);

  while(true) {
    gpio_toggle_state(&led_addr);
    delay_ms(1000);

    gpio_get_state(&led_addr, &current_led_state);
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &buff);

    // Serial Monitor
    LOG_DEBUG("ADC Reading: %f V\n", buff);

    if (current_led_state == GPIO_STATE_LOW) {
      LOG_DEBUG("LED off\n");
    } else if (current_led_state == GPIO_STATE_HIGH) {
      LOG_DEBUG("LED on\n");
    }
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");

  // Create ADC task
  tasks_init_task(run_leds, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
