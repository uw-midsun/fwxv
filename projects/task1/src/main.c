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
  LOG_DEBUG("Starting: Task1\n");
  int counter1 = 0;
  while (true) {
    LOG_DEBUG("counter1 value: %d\n", counter1);
    counter1++;
    prv_delay(1000);                            // delay for 1sec
  }
  LOG_DEBUG("Finished: Task1\n");
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Starting: Task2\n");
  int counter2 = 0;
  while (true) {
    LOG_DEBUG("counter2 value: %d\n", counter2);
    counter2++;
    prv_delay(1000);                           // delay for 1sec
  }
  LOG_DEBUG("Finished: Task2\n");
}

int main(void) {
    log_init();
    task_init();

    // Creating the tasks
    tasks_init_tasks(task1, TASK_PRIORITY(1), NULL);
    tasks_init_tasks(task2, TASK_PRIORITY(2), NULL);

    LOG_DEBUG("Program start...\n");

    // Start the scheduler
    tasks_start();

    return 0;
}
