#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "hw_timer.h"

void hardware_callback(void){
  LOG_DEBUG("STARTED");
}

TASK(HWTimer, TASK_STACK_512) {
  LOG_DEBUG("STARTING TASKS \n");
    StatusCode status = hardware_timer_init_and_start(1000, hardware_callback);
    LOG_DEBUG("code : %d\n", status);
    if (hardware_timer_init_and_start(1000, hardware_callback) != STATUS_CODE_OK) LOG_DEBUG("ERROR 1 \n");
    vTaskDelay(1000);
    if (hardware_timer_init_and_start(40,hardware_callback ) != STATUS_CODE_OK) LOG_DEBUG("ERROR 2\n");
    vTaskDelay(1000);
    if (hardware_timer_init_and_start(1, hardware_callback) != 0) LOG_DEBUG("ERROR 3\n");
    vTaskDelay(1000);
    if (hardware_timer_init_and_start(1, hardware_callback) == 0) LOG_DEBUG("ERROR 4\n");
    vTaskDelay(1000);
}



// in callbacks do log debug
// time how long between the on/off takes for the led pin itself. 
// use oscilloscope to test the timing, to see delay of the 

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");
  tasks_init_task(HWTimer, 3, NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
