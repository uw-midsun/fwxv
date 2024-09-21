#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "tasks.h"

#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "delay.h"

// #define INCLUDE_vTaskSuspend 1;
// #define INCLUDE_vTaskResume 1;

// Non blocking delay. Simply consumes cpu cycles until a given time has passed
// static void prv_delay(const TickType_t delay_ms) {
//   TickType_t curr_tick = xTaskGetTickCount();
//   while(xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(delay_ms))
//   {}
// }

bool task = false;

TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  int priority = 1;
  while (true) {
    LOG_DEBUG("Task 1 %d\n", counter1);
    counter1++;
    vTaskResume(task1->handle);
    if(counter1 % 2 == 0){
      if(priority == 1){
        priority = 2;
        vTaskPrioritySet(task1->handle, priority);
      }else{
        priority = 1;
        vTaskPrioritySet(task1->handle, priority);
      }
    }
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  int priority = 2;
  while (true) {
    LOG_DEBUG("Task 2 %d\n", counter2);
    counter2++;
    // vTaskDelay(pdMS_TO_TICKS(50));
    // prv_delay(50);
    if(counter2 % 2 == 0){
      if(priority == 2){
        priority = 1;
        vTaskPrioritySet(task2->handle, priority);
      }else{
        priority = 2;
        vTaskPrioritySet(task2->handle, priority);
      }
    }
  }
}

int main(void) {
    log_init();
    tasks_init();
    tasks_init_task(task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(task2, TASK_PRIORITY(2), NULL);

    LOG_DEBUG("Program start...\n");
    // vTaskPrioritySet(task2->handle, 1);
    // vTaskPrioritySet(task1->handle, 2);
    tasks_start();

      // prv_delay(100000);
      // vTaskPrioritySet(task2->handle, 2);
      // vTaskPrioritySet(task1->handle, 1);
      // prv_delay(100000);

    // while(true){

      // vTaskResume(task2->handle);
    // }

    return 0;
}