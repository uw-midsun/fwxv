#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "misc.h"
#include "queues.h"
#include "status.h"
#include "tasks.h"

#define LIST_SIZE 5
#define ITEM_SZ 6

#define QUEUE_LEN 6
#define BUF_SIZE (QUEUE_LEN * ITEM_SZ)

static const char s_list[LIST_SIZE][ITEM_SZ] = { "Item1", "Item2", "Item3", "Item4", "Item5" };

// Task static entities
static uint8_t s_queue1_buf[BUF_SIZE];
static Queue s_queue1 = {
  // Add parameters
  .num_items = LIST_SIZE,
  .item_size = ITEM_SZ,
  .storage_buf = s_queue1_buf,
};

TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
  StatusCode ret;
  while (true) {
    // Your code goes here
    for (int idx = 0; idx < LIST_SIZE; ++idx) {
      StatusCode status = queue_send(&s_queue1, s_list[idx], 100);
      delay_ms(100);
      if (status != STATUS_CODE_OK) LOG_DEBUG("write to queue failed!");
    }
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Task 2 initialized!\n");
  const char outstr[6];
  StatusCode ret;
  while (true) {
    // Your code goes here
    for (int idx = 0; idx < LIST_SIZE; ++idx) {
      StatusCode status = queue_receive(&s_queue1, outstr, 100);
      delay_ms(100);
      if (status != STATUS_CODE_OK) LOG_DEBUG("read from queue failed!");
    }
  }
}

int main(void) {
  log_init();
  // Initialize queues here
  queue_init(&s_queue1);

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}