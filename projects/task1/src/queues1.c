#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "misc.h"
#include "queues.h"
#include "status.h"
#include "tasks.h"

#define ITEM_SZ 6
#define QUEUE_LEN 5
#define BUF_SIZE (QUEUE_LEN * ITEM_SZ)

static const char s_list[QUEUE_LEN][ITEM_SZ] = { "Item1", "Item2", "Item3", "Item4", "Item5" };

// Task static entities
static uint8_t s_queue1_buf[BUF_SIZE];
static Queue s_queue1 = {
  // Add parameters
  .num_items = QUEUE_LEN,
  .item_size = ITEM_SZ,
  .storage_buf = s_queue1_buf,
};

TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
  StatusCode ret = STATUS_CODE_OK;
  uint32_t to_send = 0;
  while (to_send < 6) {
    // Your code goes here
    ret = queue_send(&s_queue1, &s_list[to_send], 0);
    delay_ms(100);
    to_send++;
    if (ret != STATUS_CODE_OK) {
      LOG_DEBUG("write to queue failed");
    }
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Task 2 initialized!\n");
  char outstr[ITEM_SZ];
  uint32_t to_recieve = 0;
  StatusCode ret = STATUS_CODE_OK;
  while (to_recieve < 5) {
    // Your code goes here
    ret = queue_receive(&s_queue1, &outstr, 1000);
    delay_ms(100);
    LOG_DEBUG("Received: %s\n", outstr);
    if (ret != STATUS_CODE_OK) {
      LOG_DEBUG("read from queue failed");
    }
    to_recieve++;
  }
}

int main(void) {
  log_init();
  // Initialize queues here
  tasks_init();
  queue_init(&s_queue1);

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
