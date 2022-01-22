#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "log.h"
#include "misc.h"

static StackType_t s_task_stack1;
static StaticTask_t s_task_tcb1;

static StackType_t s_task_stack2;
static StaticTask_t s_task_tcb2;


// Delay for the number of ticks specified
// Use pdMS_TO_TICKS() to convert ms to ticks
static void prv_delay(TickType_t ticks_to_wait) {
  TickType_t curr_tick = xTaskGetTickCount();
  while(xTaskGetTickCount() - curr_tick < ticks_to_wait)
  {}
}

static void task1_func(void *params) {
  int counter1 = 0;
  while (true) {
  // Your code here
  }
}

static void task2_func(void *params) {
  int counter2 = 0;
  while (true) {
  // Your code here
  }
}

int main(void) {
    // Create tasks here

    LOG_DEBUG("Program start...\n");
    vTaskStartScheduler();

    return 0;
}
