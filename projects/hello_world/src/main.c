
// #include <stdio.h>

#include "log.h"
// #include "master_task.h"
// #include "tasks.h"

// void pre_loop_init() {}

// void run_fast_cycle() {}

// void run_medium_cycle() {}

// void run_slow_cycle() {}

void increment(int *input) { 
  (*input)++;
}


int main() {

  // tasks_init();
  // log_init();
  // LOG_DEBUG("Welcome to TEST!");

  // init_master_task();

  // tasks_start();
  
  int my_int = 0;

  LOG_DEBUG("Before entering main loop");
  
  while(true){
    increment(&my_int);
    LOG_DEBUG("Hello World %d\n", my_int);
  }

  return 0;
}
