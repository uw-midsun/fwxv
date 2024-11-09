#include <stdio.h>

#include "delay.h"
#include "i_watchdog.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

TASK(task_delay, TASK_STACK_512) {
  while (true) {
    IWDG_SetValues(IWDG_Prescaler_32, 10);
    delay_ms(10000);
  }
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(task_delay, 1, NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
