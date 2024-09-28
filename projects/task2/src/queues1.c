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
//scons sim --project=task2 --platform=x86

// Task static entities
static uint8_t s_queue1_buf[BUF_SIZE];
static Queue s_queue1 = {
  // Add parameters
  .storage_buf = s_queue1_buf,
  .item_size = ITEM_SZ,
  .num_items = QUEUE_LEN,

};


TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
  StatusCode ret;
  // int num_of_strings = strlen(s_list);
  uint32_t to_send = 0;
  int ticks_to_wait = 0;
  while (true) {
    // Your code goes here
  if(to_send >= QUEUE_LEN) {
    to_send = 0;
  }

  ret = queue_send(&s_queue1, s_list[to_send], ticks_to_wait);
  
  //this copies current value of ticks_to_wait to back of the queue

  if(ret != STATUS_CODE_OK) {
    LOG_DEBUG("write to queue failed\n");
    
  }

  delay_ms(100);
  to_send++;
  }
  }
TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Task 2 initialized!\n");
  const char outstr[ITEM_SZ];
  StatusCode ret;
  while (true) {
    // Your code goes here
    ret = queue_receive(&s_queue1, outstr, 1000);
    if(ret != STATUS_CODE_OK){
      LOG_DEBUG("read from queue failed\n");
    }
    else{
      LOG_DEBUG("Received: %s\n", outstr);
    }
  }
}

int main(void) {
  log_init();
  tasks_init();
  // Initialize queues here
  queue_init(&s_queue1);
  tasks_init_task(task1, TASK_PRIORITY(1), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
