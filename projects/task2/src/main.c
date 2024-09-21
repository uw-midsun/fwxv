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
    printf("task1");
    counter1++;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
    printf("task2");
    counter2++;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int main(void) {
    log_init();
    tasks_init();
    tasks_init_task(task1, TASK_PRIORITY(1), NULL);
    tasks_init_task(task2, TASK_PRIORITY(2), NULL);
    LOG_DEBUG("Program start...\n");
    // Start the scheduler
    tasks_start();


    

    return 0;
}
