#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "tasks.h"
#include "queues.h"
#include "status.h"
#include "delay.h"

#include "log.h"
#include "misc.h"


#define LIST_SIZE 5
#define ITEM_SZ 6

#define QUEUE_LEN 5
#define BUF_SIZE (QUEUE_LEN * ITEM_SZ)

static const char s_list[LIST_SIZE][ITEM_SZ] = {
	"Item1",
	"Item2",
	"Item3",
	"Item4",
	"Item5"
};

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
    for(int i = 0; i < LIST_SIZE; i++) {
      ret = queue_send(&s_queue1, s_list[i], 0);
      if (ret) {
        LOG_DEBUG("Failed to send to queue\n");
      }
      delay_ms(1000);
    }
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Task 2 initialized!\n");
  const char outstr[6];
  StatusCode ret;
  while (true) {
    // Your code goes here
    ret = queue_receive(&s_queue1, outstr, 0);
    if (ret) {
      LOG_DEBUG("Failed to receive from queue\n");
    } else {
      LOG_DEBUG("Received %s from queue\n", outstr);
    }
    delay_ms(1000);
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
