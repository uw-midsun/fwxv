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
#include "delay.h"
#include "ads1115.h"
#include "gpio_it.h"
#include "i2c.h"

GpioAddress led_addr = {
  .port = GPIO_PORT_B,
  .pin = 3
};

GpioAddress ready_pin = {
      .port = GPIO_PORT_B,
      .pin = 0
    };

TASK(leds_task, TASK_STACK_512){
  
  
  while (true) {
    // #ifdef MS_PLATFORM_X86
    LOG_DEBUG("blink\n");
    // #endif
    gpio_toggle_state(&led_addr);
    delay_ms(100);
    
  }
}
TASK(ben_task, TASK_STACK_512){
  
  ADS1115_Config config = {
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_port = ADS1115_I2C_PORT,
  };
  float reading;
  while (true){
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &reading);
    LOG_DEBUG("%f\n", reading);
  }

}


int main() {
  
  tasks_init();
  log_init();
  gpio_init();
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  tasks_init_task(leds_task, 2, NULL);
  tasks_init_task(ben_task, 2, NULL);
  tasks_start();
  


  return 0;
}
