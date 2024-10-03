#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "tasks.h"
#include "queues.h"
#include "status.h"
#include "delay.h"

#include "log.h"
#include "misc.h"

#define ITEM_SZ 6
#define QUEUE_LEN 5
#define BUF_SIZE (QUEUE_LEN * ITEM_SZ)

#define NUM_ITEMS 5
#define ITEM_SIZE sizeof(char) * ITEM_SZ  // Each item is an array of characters

static const char s_list[QUEUE_LEN][ITEM_SZ] = {
  "Item1",
  "Item2",
  "Item3",
  "Item4",
  "Item5"
};

// Task static entities
static uint8_t s_queue1_buf[BUF_SIZE];
static Queue s_queue1 = {
  .num_items = NUM_ITEMS,
  .item_size = ITEM_SIZE,
  .storage_buf = s_queue1_buf,
};

TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
  StatusCode ret;
  
  while (true) {
    for (uint8_t i = 0; i < NUM_ITEMS; ++i) {
      // Attempt to send an item to the queue
      ret = queue_send(&s_queue1, s_list[i], 0);
      
      // Check if the queue push was successful
      if (ret != STATUS_CODE_OK) {
        LOG_DEBUG("write to queue failed\n");
      }

      // Delay for 100 ms before sending the next item
      delay_ms(100);
    }
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Task 2 initialized!\n");
  char outstr[ITEM_SZ];
  StatusCode ret;

  while (true) {
    // Attempt to receive an item from the queue
    ret = queue_receive(&s_queue1, &outstr, UINT32_MAX);
    
    // Check if the dequeue was successful
    if (ret == STATUS_CODE_OK) {
      LOG_DEBUG("Received: %s\n", outstr);
    } else {
      LOG_DEBUG("read from queue failed\n");
    }
  }
}

int main(void) {
  log_init();

  // Initialize the task system
  tasks_init();  // Initialize task system properly

  // Initialize the queue
  queue_init(&s_queue1);

  // Initialize tasks
  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
