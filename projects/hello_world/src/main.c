#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include "delay.h"

#define DELAY_TIME 1000

static int prv_inc_count(int *input) {
  (*input)++;
  return *input;
}

TASK(hello_world, TASK_STACK_512) {
  static int count = 0;
  while (true) {
    LOG_DEBUG("%i\n",prv_inc_count(&count));
    LOG_DEBUG("Hello world!\n");  
    delay_ms(DELAY_TIME);
  }
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  init_master_task();
  tasks_init_task(hello_world, TASK_PRIORITY(2), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
