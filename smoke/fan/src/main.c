#include <stdio.h>

#include "gpio.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include "pwm.h"

int main() {
  gpio_init();
  pwm_init(PWM_TIMER_3, 40);
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");
  pwm_set_pulse(PWM_TIMER_3, 40);
  pwm_set_dc(PWM_TIMER_3, 100);
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
