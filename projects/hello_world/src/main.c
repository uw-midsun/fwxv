#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  int my_int = 0;
  tasks_init();
  log_init();

  while (true) {
    my_int++;
    LOG_DEBUG("Hello World! %d\n", my_int);
  }

  return 0;
}
