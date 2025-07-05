// Fimware 102 TASKS - ETHAN GAN 
#include <stdbool.h>
#include <stdint.h>

// Blocking API included here 
#include "FreeRTOS.h"
#include "tasks.h"

#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "delay.h"

// // Non blocking delay. Simply consumes cpu cycles until a given time has passed
// static void prv_delay(const TickType_t delay_ms) {
//   TickType_t curr_tick = xTaskGetTickCount();
//   while(xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(delay_ms))
//   {}
// }

TASK(task1, TASK_STACK_512) {
  int counter1 = 0;
  while (true) {
  // Your code here
// Print the name of the task and the counter value using LOG_DEBUG
// Increment the counter
// Call the prv_delay function for 1 second

    LOG_DEBUG("Task 1 Counter Value: %d\n", counter1);
    counter1++; 
    // prv_delay(1000);
    vTaskDelay(pdMS_TO_TICKS(1000));// Figure out how to import
  }
  vTaskDelete(NULL);
}

TASK(task2, TASK_STACK_512) {
  int counter2 = 0;
  while (true) {
  // Your code here
    LOG_DEBUG("Task 2 Counter Value: %d\n", counter2);
    counter2++; 
    // prv_delay(1000);
    vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);

}

int main(void) {
    log_init();

    tasks_init();
    // Create tasks here
    // TASK 1 TASK IMPLEMENTATION
    // SAME PRIORITY
    // tasks_init_task(task1, TASK_PRIORITY(2), NULL);
    // tasks_init_task(task2, TASK_PRIORITY(2), NULL);

    // TASK 2 BLOCKING APIS
    tasks_init_task(task1, TASK_PRIORITY(2), NULL);
    tasks_init_task(task2, TASK_PRIORITY(1), NULL);


    LOG_DEBUG("Program start...\n");
    // Start the scheduler
    tasks_start();


    return 0;
}
