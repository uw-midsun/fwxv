#include <stdio.h>

#include "ads1115.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"

GpioAddress ready_pin = {
  .port = GPIO_PORT_B,
  .pin = GPIO_Pin_0,
};

GpioAddress led_addr = {
  .port = GPIO_PORT_B,
  .pin = 3,
};

TASK(led_toggle, TASK_STACK_512) {
  gpio_init_pin(&led_addr, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  while (true) {
    LOG_DEBUG("led toggle!\n");
    gpio_toggle_state(&led_addr);
    delay_s(1);
  }
}

TASK(adc_read, TASK_STACK_512) {
  ADS1115_Config config = {
    .i2c_port = ADS1115_I2C_PORT,
    .i2c_addr = ADS1115_ADDR_GND,
    .handler_task = adc_read,
    .ready_pin = &ready_pin,
  };

  ads1115_init(&config, ADS1115_ADDR_GND, &ready_pin);

  float reading;
  while (true) {
    ads1115_read_converted(&config, ADS1115_CHANNEL_0, &reading);
    LOG_DEBUG("reading val: %f\n", reading);
    delay_ms(100);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();

  tasks_init_task(led_toggle, TASK_PRIORITY(2), NULL);
  tasks_init_task(adc_read, TASK_PRIORITY(2), NULL);

  tasks_start();
  return 0;
}