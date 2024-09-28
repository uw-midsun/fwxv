#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "tasks.h"

#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "delay.h"




// scons sim --project=<your_project> --platform=x86


// Non blocking delay. Simply consumes cpu cycles until a given time has passed
static void prv_delay(const TickType_t delay_ms) {
  TickType_t curr_tick = xTaskGetTickCount();
  while(xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(delay_ms))
  {}
}

TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  while (true) {
  // Your code here
  LOG_DEBUG("task1%d\n",counter1);
  counter1++;
  prv_delay(1000);


  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {

  // Your code here
  LOG_DEBUG("task2%d\n",counter2);
  counter2++;
  prv_delay(1000);



  }
}

int main(void) {
    
    // Create tasks here
    log_init();
    tasks_init();

    tasks_init_task(task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(task2, TASK_PRIORITY(2), NULL);
    LOG_DEBUG("Program start...\n");
    // Start the scheduler


    tasks_start();

    return 0;
}
