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

// Task static entities
static uint8_t s_queue1_buf[BUF_SIZE];
static Queue s_queue1 = {
  // Add parameters
  .num_items = QUEUE_LEN,
  .item_size = ITEM_SZ,
  .storage_buf = s_queue1_buf,
};

static const char s_list[QUEUE_LEN][ITEM_SZ] = {
"Item1",
"Item2",
"Item3",
"Item4",
"Item5"
};



TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 initialized!\n");


  while (true) {
    // Your code goes here
    for(int i = 0; i < (int)sizeof(s_list); i++){
      queue_send(&s_queue1, s_list[i], 0);
      LOG_DEBUG("pee poo: %s\n", s_list[i]);
      delay_ms(100);
      if(statuscode(queue_send(&s_queue1, s_list[i], 0)) != STATUS_CODE_OK){
        LOG_DEBUG("Write to queue failed");
        delay_ms(1000);
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
    for(int i = 0; i < (int)sizeof(s_list); i++){
      queue_receive(&s_queue1, &s_queue1_buf[i], 0);
      LOG_DEBUG("Item: %s\n", &s_queue1_buf[i]);
      delay_ms(100);
         if(statuscode(queue_receive(&s_queue1, &s_queue1_buf[i], 0)) != STATUS_CODE_OK){
        LOG_DEBUG("Write to queue failed");
        delay_ms(1000);
      }
    }
  }
}

int main(void) {
  log_init();
  tasks_init();
  queue_init(&s_queue1);
  // Initialize queues here

  tasks_init_task(task1, TASK_PRIORITY(3), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}