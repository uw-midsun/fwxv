#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include <unistd.h>

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  for (int my_int = 0; ; my_int++) {
    LOG_DEBUG("Hello World %d\n", my_int);
    sleep(1);
  }

  LOG_DEBUG("exiting main?");
  return 0;
}
