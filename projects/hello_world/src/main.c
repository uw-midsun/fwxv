#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

int increment_int(int *input);

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {
  static int my_int = 0;
  my_int = increment_int(&my_int);
  LOG_DEBUG("Hello World %d\n", my_int);
}

int increment_int(int *input) {
  return *input + 1;
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
