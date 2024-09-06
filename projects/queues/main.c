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
};


TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");
  StatusCode ret;
  while (true) {
    // Your code goes here
    int counter = 0;
    queue_send(&s_queue1, &s_list[counter], 0);
    counter++;
    delay(100);
    if(STATUS_CODE_OK){
      LOG_DEBUG("write to queue failed");
    }
    
  }
}

TASK(task2, TASK_STACK_512) {
  string_t item = "";
  LOG_DEBUG("Task 2 initialized!\n");
  const char outstr[ITEM_SZ];
  StatusCode ret;
  while (true) {
    // Your code goes here
    queue_receive(&s_queue1, &string_t, 1000);
    LOG_DEBUG("Received: %u\n",item);
  } 
}

int main(void) {
  log_init();
  // Initialize queues here

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  queue_init(&s_queue1);
  tasks_start();

  return 0;
}
