#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "master_task.h"
#include "tasks.h"

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int my_int=0;

int main() {
  tasks_init();
  log_init();

  while (true){
    my_int++;
    LOG_DEBUG("Hello World! %d\n\n", my_int);
    sleep(1);
    

  }
  

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}