#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"

// Non blocking delay. Simply consumes cpu cycles until a given time has passed
static void prv_delay(const TickType_t delay_ms) {
  TickType_t curr_tick = xTaskGetTickCount();
  while (xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(delay_ms)) {
  }
}

TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  while (true) {
    // Your code here
    LOG_DEBUG("task1 %d", counter1);
    counter1++;
    prv_delay(pdMS_TO_TICKS(1000));
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
    // Your code here
    LOG_DEBUG("task2 %d", counter2);
    counter2++;
    prv_delay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
  tasks_init();
  log_init();

  tasks_init_task(task1, TASK_PRIORITY(1), NULL);
  tasks_init_task(task2, TASK_PRIORITY(1), NULL);
  // Create tasks here

  LOG_DEBUG("Program start...\n");
    // Start the scheduler
  tasks_start();

  return 0;
}
