#include "tasks.h"

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"

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
    LOG_DEBUG("Task 1: %d\n", counter1++);
    delay_ms(1000);
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
    // Your code here
    LOG_DEBUG("Task 2: %d\n", counter2++);
    prv_delay(1000);
  }
}

int main(void) {
  log_init();
  // Create tasks here

  // priority 2 should give processor back to logs and task2 since task 1 has blocking function call
  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(1), NULL);

  LOG_DEBUG("Program start...\n");
  // Start the scheduler
  tasks_start();

  return 0;
}
