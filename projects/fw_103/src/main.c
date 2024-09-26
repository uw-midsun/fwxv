#include <stdio.h>
#include "gpio.h"

#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include "delay.h"
#include "ads1115.h"

TASK(task1, TASK_STACK_512) {
  float reading;
  int counter1 = 0;
  GpioAddress led_addr = {
    .port = GPIO_PORT_B,
    .pin = 3,
  };

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

  while (true) {
    counter1++;
    // gpio_toggle_state(&led_addr);
    // delay_ms(1); //blocking -> lets other things run
    // //prv_delay(1) //nonblocking -> consumes clock cycles
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &reading);

    LOG_DEBUG("Voltage: %f\n", reading);

  }
}


int main() {
  log_init();
  gpio_init();
  tasks_init();
  tasks_init_task(task1, TASK_PRIORITY(1), NULL);
  LOG_DEBUG("Welcome to TEST!");


  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}