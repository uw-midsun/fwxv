#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "hw_timer.h"
#include "interrupt.h"
#include "log.h"
#include "tasks.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

void pre_loop_init() {}

TASK(hw_timer_task, TASK_STACK_512) {
  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  }

  while (true) {
    gpio_toggle_state(&leds[0]);
    gpio_toggle_state(&leds[3]);
    LOG_DEBUG("HW Delay");
    hw_timer_delay_us(100);
  }
}

int main() {
  tasks_init();
  interrupt_init();
  gpio_init();
  hw_timer_init();
  log_init();

  tasks_init_task(hw_timer_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  return 0;
}
