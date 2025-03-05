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
  // Add parameters
  .num_items = QUEUE_LEN,
  .item_size = ITEM_SZ,
  .storage_buf = s_queue1_buf,
};

// static Queue s_my_queue = {
//   .num_items = NUM_ITEMS,
//   .item_size = ITEM_SIZE,
//   .storage_buf = s_queue_buf,
// };
// queue_send(&s_my_queue, &to_send, 1000);
// queue_receive(&s_my_queue, &receive, 1000);

TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
  StatusCode ret;
  while (true) {
    // Your code goes here
    
    // Loop through the list of strings, pushing them to the queue with queue_send (ticks_to_wait = 0)
    // Delay for 100 ms between queue pushes
    // Get the status from the above method, and LOG_DEBUG ”write to queue failed”) it if it is != STATUS_CODE_OK
    char *currString = 0;

    for (int i=0; i<QUEUE_LEN; i++){
      currString = s_list[i];
      ret = queue_send(&s_queue1, currString, 0);
      // LOG_DEBUG("Index %u: %p is %s\n", i, currString, currString);
      delay_ms(100);
      if (ret != STATUS_CODE_OK){
        LOG_DEBUG("write to queue failed\n");
      }
    }
  }
}

TASK(task2, TASK_STACK_512) {
  LOG_DEBUG("Task 2 initialized!\n");
  const char outstr[ITEM_SZ];
  StatusCode ret;
  while (true) {
    // Your code goes here

    // Receive the strings from the queue to the provided buffer
    // If the dequeue was successful then LOG_DEBUG the string, otherwise print (“read from queue failed”)
    ret = queue_receive(&s_queue1, outstr, 120); // if this wait is too short and data is not in the queue, it will return error status code
    // delay_ms(100);
    if (ret != STATUS_CODE_OK){
      LOG_DEBUG("read from queue failed\n");
    }
    else{
      LOG_DEBUG("Value from queue: %s\n", outstr);
    }
  }
}

int main(void) {
  log_init();
  tasks_init();

  // Initialize queues here
  queue_init(&s_queue1);

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
