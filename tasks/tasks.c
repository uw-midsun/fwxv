#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "tasks.h"

#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "delay.h"


TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  while (true) {
    // Your code here
    LOG_DEBUG("Task1 counter value = %d\n", counter1);
    ++counter1;
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
    // Your code here
    LOG_DEBUG("Task2 counter value = %d\n", counter2);
    ++counter2;
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

int main(void) {
    log_init();
    // Create tasks here
    tasks_init_task(task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(task2, TASK_PRIORITY(2), NULL);
    LOG_DEBUG("Program start...\n");

    // Start the scheduler
    vTaskStartScheduler();

    return 0;
}
