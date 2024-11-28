#include <stdio.h>
#include "log.h"
#include "tasks.h"
#include "hw_timer.h"
#include "soft_timer.h"

// in callbacks do log debug
// time how long between the on/off takes for the led pin itself. 
  // use oscilloscope to test the timing, to see delay of the 
static int i= 0;

TASK(HWTimer, TASK_STACK_512) {
    if (hardware_timer_init_and_start(1, hardware_callback) != 0) LOG_DEBUG("ERROR");
    //if (hardware_timer_init_and_start(40,hardware_callback ) != 0) LOG_DEBUG("ERROR");
    //if (hardware_timer_init_and_start(1, hardware_callback) != 0) LOG_DEBUG("ERROR");
    //if (hardware_timer_init_and_start(1, hardware_callback) == 0) LOG_DEBUG("ERROR");
}

void hardware_callback(void){
  i++;
  if (i % 1000000 == 0){
    LOG_DEBUG("%d\n", i/100000);
  }
}


int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");
  tasks_init_task(HWTimer, 1, NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
