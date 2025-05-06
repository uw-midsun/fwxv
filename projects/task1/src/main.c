#include <stdbool.h>
#include <stdint.h>
#include "tasks.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "delay.h"

// Non-blocking delay (CPU busy wait)
static void prv_delay(const TickType_t delay_ms) {
  TickType_t curr_tick = xTaskGetTickCount();
  while (xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(delay_ms)) {
    
  }
}

// Task 1 definition
TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  while (true) {
      LOG_DEBUG("Task1 - Counter: %d\n", counter1);
      counter1++;
      prv_delay(1000); // 1 second non-blocking delay
    }
}

// Task 2 definition
TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
      LOG_DEBUG("Task2 - Counter: %d\n", counter2);
      counter2++;
      vTaskDelay(pdMS_TO_TICKS(3000)); // 3 second blocking delay
    }
} 

// Main function
int main(void) {
    log_init();
    LOG_DEBUG("Program start...");
    tasks_init(); 

    tasks_init_task(task1, 1, NULL); // priority 1
    tasks_init_task(task2, 2, NULL); // priority 2

    tasks_start(); // Scheduler starts here

    // safe guard
    while (1);
    return 0;
}