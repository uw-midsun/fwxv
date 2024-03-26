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
  .item_size = ITEM_SZ,
  .num_items = QUEUE_LEN,
  .storage_buf = s_queue1_buf,
};

TASK(task1, TASK_STACK_512) {
  StatusCode ret;
  u_int32_t to_send = 0;
  LOG_DEBUG("Task 1 initialized!\n");
  while (true) {
    ret = queue_send(&s_queue1, s_list[to_send], 100);
    if (ret == STATUS_CODE_OK) {
      LOG_DEBUG("task1: %s\n", s_list[to_send]);
    } else {
      LOG_DEBUG("write to queue failed\n");
    }
    delay_ms(100);
  }
}

TASK(task2, TASK_STACK_512) {
  const char outstr[ITEM_SZ];
  StatusCode ret;
  LOG_DEBUG("Task 2 initialized!\n");
  while (true) {
    ret = queue_receive(&s_queue1, outstr, 500);
    if (ret == STATUS_CODE_OK) {
      LOG_DEBUG("task2: %s\n", outstr);
    } else {
      LOG_DEBUG("read from queue failed\n")
    }
  }
}

int main(void) {
  log_init();
  queue_init(&s_queue1);
  tasks_init();

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
