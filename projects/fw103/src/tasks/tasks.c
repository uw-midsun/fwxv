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
    LOG_DEBUG("Task: %s | Counter: %d\n", "task1", counter1);
    counter1++;
    prv_delay(2000);
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
    LOG_DEBUG("Task: %s | Counter: %d\n", "task2", counter2);
    counter2++;
    delay_ms(1000);  // task is in blocked state for 1s before
  }
}

int main(void) {
  log_init();
  // Create tasks here
  tasks_init_task(task1, TASK_PRIORITY(1), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  // Start the scheduler
  tasks_start();

  return 0;
}
