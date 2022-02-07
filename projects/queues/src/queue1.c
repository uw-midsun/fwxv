#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "log.h"
#include "misc.h"


#define LIST_SIZE 5
#define ITEM_SIZE 5

#define QUEUE_LENGTH 5
#define BUF_SIZE (QUEUE_LENGTH * ITEM_SIZE)

static const char s_list[ITEM_SIZE][LIST_SIZE] = {
	"Item1",
	"Item2",
	"Item3",
	"Item4",
	"Item5"
};

// Queue static entities
static StaticQueue_t s_queue;
static uint8_t s_queue_buf[BUF_SIZE];
static QueueHandle_t s_queue_handle;

// Task static entities
static StackType_t s_task_stack1;
static StaticTask_t s_task_tcb1;

static StackType_t s_task_stack2;
static StaticTask_t s_task_tcb2;


static void task1_func(void *params) {
  LOG_DEBUG("Task 1 initialized!\n");
  BaseType_t status;
  const TickType_t delay_ticks = pdMS_TO_TICKS(100);
	
  uint8_t index = 0;
  while (true) {
    //vTaskDelay(delay_ticks);
    status = xQueueSendToBack( s_queue_handle, s_list[index], 0);
    if (status != pdPASS) {
      LOG_DEBUG("Error writing to queue %d\n", index);
      continue;
    }
    if (++index == 5) {
      index = 0;
    }
  }

}

static void task2_func(void *params) {
  LOG_DEBUG("Task 2 initialized!\n");
  BaseType_t status;
  const TickType_t delay_ticks = pdMS_TO_TICKS(100);
  const char outstr[5];
  while (true) {
    //vTaskDelay(delay_ticks);
    status = xQueueReceive( s_queue_handle, outstr, 0);
    if (status != pdPASS) {
      LOG_DEBUG("Error reading from queue\n");
      continue;
    }
    LOG_DEBUG("Received string: %s in task 2\n", outstr);
  }
}

int main(void) {
    s_queue_handle = xQueueCreateStatic(
       QUEUE_LENGTH,
       ITEM_SIZE,
       s_queue_buf,
       &s_queue);
        
    xTaskCreateStatic(
        task1_func,
        "task 1",
        configMINIMAL_STACK_SIZE*2,
        NULL,
        tskIDLE_PRIORITY + 1,
        &s_task_stack1,
        &s_task_tcb1
    );
    xTaskCreateStatic(
        task2_func,
        "task 2",
        configMINIMAL_STACK_SIZE*2,
        NULL,
        tskIDLE_PRIORITY + 1
        &s_task_stack2,
        &s_task_tcb2
    );

    LOG_DEBUG("Program start...\n");
    vTaskStartScheduler();

    return 0;
}
