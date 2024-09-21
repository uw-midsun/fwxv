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
  .num_items = QUEUE_LEN,
  .item_size = ITEM_SZ,
  .storage_buf = s_queue1_buf,
};


TASK(task1, TASK_STACK_512) {
  StatusCode status = STATUS_CODE_OK;
  LOG_DEBUG("Task 1 initialized!\n");
  int counter = 0;
  StatusCode ret;
  while (true) {

    status = queue_send(&s_queue1, &s_list[counter%5], 0);
    if (status != STATUS_CODE_OK){
      LOG_DEBUG("write to queue failed");
    }
    counter++;
    delay_ms(100);
  }
  
}

TASK(task2, TASK_STACK_512) {
  StatusCode status = STATUS_CODE_OK;
  LOG_DEBUG("Task 2 initialized!\n");
  const char outstr[ITEM_SZ];
  while (true) {
    status = queue_receive(&s_queue1, &s_queue1_buf, 0);
    if (status != STATUS_CODE_OK){
      // LOG_DEBUG("read from queue failed");
    }
    delay_ms(50);
    LOG_DEBUG("Received: %s\n", s_queue1_buf);
  }
}

int main(void) {
  tasks_init();
  log_init();
  // Initialize queues here
  queue_init(&s_queue1);

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}