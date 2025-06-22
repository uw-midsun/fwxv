#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "tasks.h"

#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "delay.h"

// Non blocking delay. Simply consumes cpu cycles until a given time has passed
static void prv_delay(const TickType_t delay_ms) {
  TickType_t curr_tick = xTaskGetTickCount();
  while(xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(delay_ms))
  {}
}

TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  while (true) {
  counter1++;
  LOG_DEBUG("task: %s, counter: %d \n", pcTaskGetName(NULL), counter1);
  prv_delay(0);
  delay_ms(1);
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
  counter2++;
  LOG_DEBUG("task: %s, counter: %d \n", pcTaskGetName(NULL), counter2);
  prv_delay(0);
  delay_ms(2);
  }
}

int main(void) {
    log_init();
    tasks_init();

    // Create tasks here
    
    tasks_init_task(task1, TASK_PRIORITY(2), NULL);
    tasks_init_task(task2, TASK_PRIORITY(3), NULL);

    LOG_DEBUG("Program start...\n");
    // Start the scheduler
    tasks_start();
    return 0;
}

