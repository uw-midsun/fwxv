#include "tasks.h"

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"

TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  while (true) {
    LOG_DEBUG("Executing task1: %d\n", counter1);
    counter1++;
    delay_s(1);
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
    LOG_DEBUG("Executing task2: %d\n", counter2);
    counter2++;
    delay_s(2);
  }
}

int main(void) {
  log_init();
  // Create tasks here
  tasks_init();

  tasks_init_task(task1, TASK_PRIORITY(1), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  // Start the scheduler
  tasks_start();

  return 0;
}
