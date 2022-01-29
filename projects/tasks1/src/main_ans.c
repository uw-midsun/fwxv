#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "log.h"
#include "misc.h"

static StackType_t s_task_stack1[configMINIMAL_STACK_SIZE];
static StaticTask_t s_task_tcb1;

static StackType_t s_task_stack2[configMINIMAL_STACK_SIZE];
static StaticTask_t s_task_tcb2;

static void prv_delay(TickType_t tick) {
  TickType_t curr_tick = xTaskGetTickCount();
  while(xTaskGetTickCount() - curr_tick < pdMS_TO_TICKS(tick))
  {}
}

static void task1_func(void *params) {
  int counter = 0;
  while (true) {
    LOG_DEBUG("Task 1 called: %d!\n", counter);
    counter++;
    prv_delay(pdMS_TO_TICKS(1000));
  }
}

static void task2_func(void *params) {
  int counter = 0;
  while (true) {
    LOG_DEBUG("Task 2 called: %d!\n", counter);
    counter++;
    prv_delay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
    xTaskCreateStatic(
        task1_func,
        "task 1",
        configMINIMAL_STACK_SIZE*2,
        NULL,
        tskIDLE_PRIORITY + 1,
        s_task_stack1,
        &s_task_tcb1
    );
    xTaskCreateStatic(
        task2_func,
        "task 2",
        configMINIMAL_STACK_SIZE*2,
        NULL,
        tskIDLE_PRIORITY + 2,
        s_task_stack2,
        &s_task_tcb2
    );

    LOG_DEBUG("Program start...\n");
    vTaskStartScheduler();

    return 0;
}
