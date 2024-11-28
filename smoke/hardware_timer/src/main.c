#include <stdio.h>
#include "log.h"
#include "tasks.h"

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void hardware_callback(void){
  static i = 0;
  LOG_DEBUG("%d", i);
  i++;
}

// in callbacks do log debug
// time how long between the on/off takes for the led pin itself. 
  // use oscilloscope to test the timing, to see delay of the 


static void private_callback(void){
      LOG_DEBUG("Callback triggered!");
      
}

TASK(timer_task, TASK_STACK_512){

}


int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");
  tasks_init_task(timer_task, 1, NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
