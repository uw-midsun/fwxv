#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

// void pre_loop_init() {}

// void run_fast_cycle() {}

// void run_medium_cycle() {}

// void run_slow_cycle() {}

int main() {
  // tasks_init();
  // log_init();
  // LOG_DEBUG("Welcome to TEST!");

  // init_master_task();

  // tasks_start();

  // LOG_DEBUG("exiting main?");

  int seconds = 0;
  while(true) {
    LOG_DEBUG("Hello, World! %d\n", seconds);
    seconds++;

  }
  

  return 0;
}
