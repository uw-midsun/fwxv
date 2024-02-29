#include <stdio.h>

#include "gpio.h"
#include "log.h"
#include "master_task.h"
#include "pwm.h"
#include "tasks.h"

int main() {
  gpio_init();
  pwm_init(PWM_TIMER_3, 40);
  log_init();
  LOG_DEBUG("Welcome to TEST!");
  pwm_set_pulse(PWM_TIMER_3, 40);
  pwm_set_dc(PWM_TIMER_3, 100);

  LOG_DEBUG("exiting main?");
  return 0;
}
