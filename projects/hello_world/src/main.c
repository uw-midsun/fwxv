#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "log.h"
#include "master_task.h"
#include "tasks.h"

static void prv_delay(const TickType_t delay_ms) {
  TickType_t curr_tick = xTaskGetTickCount();
  while(xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(delay_ms))
  {}
}
TASK(hello, TASK_STACK_512){
    while(true){
      printf("HELLO");
      prv_delay(100);
    }
    
}

TASK(count, TASK_STACK_512){
    int counter = 0;
    while(true){
      counter++;
      printf("%d\n", counter);
      prv_delay(100);
    }
}
void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");
  
  tasks_init_task(hello, TASK_PRIORITY(1), NULL);
  tasks_init_task(count, TASK_PRIORITY(1), NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
