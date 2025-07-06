/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and read from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
*/
// Completed by Ethan Gan 
#include "gpio.h"
#include "delay.h"

#include "ads1115.h"

#include <stdio.h>
#include "log.h"
#include "tasks.h"


GpioAddress led_addr = {
  // PB5 = Port B, Pin 5
  // Green LED from the FW103 picture
  .port = GPIO_PORT_B,
  .pin = 5,
};

TASK(task1, TASK_STACK_512) {
    GpioAddress ready_pin = {
    .port = GPIO_PORT_B,
    .pin = GPIO_Pin_0,
  };
  ADS1115_Config config = {
    // ig handler task is not "FW103 Final Task: Blink Green LED 1 blink per second",
    .handler_task = (Task *)xTaskGetCurrentTaskHandle(),
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
    .ready_pin = &ready_pin,
  };
  float voltage;
  StatusCode status = ads1115_init(&config, config.i2c_addr, config.ready_pin); // start the ads program
  // review struct syntax later
  if (status!=STATUS_CODE_OK) {
    LOG_DEBUG("Failed to start ADS1115\n");
      vTaskDelete(NULL);
  }

  LOG_DEBUG("blink started\n");

  while(true) {
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &voltage); 
    // I still don't get the channel parameter, it looks like it is unused
  
    LOG_DEBUG("Voltage reading: %f V\n", voltage);
    gpio_toggle_state(&led_addr);
    LOG_DEBUG("blink\n");

    delay_ms(1000);
  }
  vTaskDelete(NULL);
  
}

int main() {
  tasks_init();
  log_init();
  gpio_init(); // already done
  
  tasks_init_task(task1, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Welcome to FW 103!\n");



  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
