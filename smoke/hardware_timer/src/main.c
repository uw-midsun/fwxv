#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "../stm32f10x_hw_timer.h"


TASK(HWTimer, TASK_STACK_512) {
  while(true) {
    if (hardware_timer_init_and_start(1000, hardware_callback) != 0) LOG_DEBUG("ERROR");
    if (hardware_timer_init_and_start(40,hardware_callback ) != 0) LOG_DEBUG("ERROR");
    if (hardware_timer_init_and_start(1, hardware_callback) != 0) LOG_DEBUG("ERROR");
    if (hardware_timer_init_and_start(1, hardware_callback) == 0) LOG_DEBUG("ERROR");
  }
}

void hardware_callback(void){
  static i = 0;
  LOG_DEBUG("%d", i);
  i++;  


}

// in callbacks do log debug
// time how long between the on/off takes for the led pin itself. 
  // use oscilloscope to test the timing, to see delay of the 

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
