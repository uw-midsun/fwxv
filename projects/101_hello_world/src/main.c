#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "notify.h"
#include "tasks.h"

void pre_loop_init() {}

void run_fast_cycle() {}

int increment(int i) {
  i++;
  return i;
}

TASK(increment_task, TASK_STACK_512) {
  int my_int = 0;
  while (true) {
    LOG_DEBUG("Hello World %d\n", my_int);
    my_int = increment(my_int);
    delay_ms(100);
  }
}

int main() {
  tasks_init();
  gpio_init();
  log_init();

  tasks_init_task(increment_task, TASK_PRIORITY(2), NULL);

  tasks_start();
  LOG_DEBUG("Hello World Start...\n");

  return 0;
}
