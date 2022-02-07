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

static void task1_func(void *params) {
  TickType_t curr_tick = xTaskGetTickCount();
  while (true) {
    LOG_DEBUG("Task 1 called!\n");
    xTaskDelayUntil(&curr_tick, pdMS_TO_TICKS(100));
  }
}

static void task2_func(void *params) {
  TickType_t curr_tick = xTaskGetTickCount();
  while (true) {
    LOG_DEBUG("Task 2 called!\n");
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

int main(void) {
    xTaskCreateStatic(
        task1_func,
        "task 1",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        &s_task_stack1,
        &s_task_tcb1
    );
    xTaskCreateStatic(
        task2_func,
        "task 2",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        &s_task_stack2,
        &s_task_tcb2
    );

    LOG_DEBUG("Program start...\n");
    vTaskStartScheduler();

    return 0;
}
