#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "queues.h"
#include "status.h"
#include "tasks.h"

#include "log.h"
#include "misc.h"

#define LIST_SIZE 5
#define ITEM_SZ 6

#define QUEUE_LEN 5
#define BUF_SIZE (QUEUE_LEN * ITEM_SZ)

static const char s_list[LIST_SIZE][ITEM_SZ] = { "Item1", "Item2", "Item3", "Item4", "Item5" };

static uint8_t s_queue1_buf[BUF_SIZE];
static Queue s_queue1 = {
  .num_items = LIST_SIZE, .item_size = ITEM_SZ, .storage_buf = s_queue1_buf
};

TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
  int current_item = 0;
  StatusCode ret;
  while (true) {
    if (current_item < LIST_SIZE) {
      ret = queue_send(&s_queue1, &s_list[current_item], 1000);
    }
    delay_ms(1000);
    if (ret == STATUS_CODE_OK) {
      current_item++;
    } else {
      LOG_DEBUG("write to queue failed\n");
    }
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Task 2 initialized!\n");
  char outstr[6];
  StatusCode ret;
  while (true) {
    ret = queue_receive(&s_queue1, &outstr, 1000);
    delay_ms(500);
    if (ret == STATUS_CODE_OK) {
      LOG_DEBUG("read in: %s \n", outstr);
    } else {
      LOG_DEBUG("read for queue failed \n");
    }
  }
}

int main(void) {
  log_init();

  queue_init(&s_queue1);

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
