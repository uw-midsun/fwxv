#include "log.h"
#include "queues.h"
#include "tasks.h"
#include "delay.h"

#define NUM_ITEMS 5
#define ITEM_SIZE 6  // Enough for "ItemX" + '\0'

// List of strings to send
static const char s_list[NUM_ITEMS][ITEM_SIZE] = {
  "Item1",
  "Item2",
  "Item3",
  "Item4",
  "Item5"
};

// Queue buffer and setup
static uint8_t s_queue_buf[NUM_ITEMS * ITEM_SIZE];
static Queue s_my_queue = {
  .num_items = NUM_ITEMS,
  .item_size = ITEM_SIZE,
  .storage_buf = s_queue_buf,
};

// Task 1: Send strings to the queue
TASK(task1, TASK_STACK_512) {
  size_t index = 0;
  StatusCode ret;

  while (true) {
    LOG_DEBUG("Sending: %s (index %d)\n", s_list[index], (int)index);
    ret = queue_send(&s_my_queue, s_list[index], 0);  // ticks_to_wait = 0

    if (ret != STATUS_CODE_OK) {
      LOG_DEBUG("Queue send failed! Code: %d\n", ret);
    } else {
      LOG_DEBUG("Queue send succeeded: %s\n", s_list[index]);
    }

    index = (index + 1) % NUM_ITEMS;
    delay_ms(1000);
  }
}

// Task 2: Receive and log strings from the queue
TASK(task2, TASK_STACK_512) {
  char outstr[ITEM_SIZE] = {0};
  StatusCode ret;

  while (true) {
    ret = queue_receive(&s_my_queue, outstr, 0);  // ticks_to_wait = 0

    if (ret == STATUS_CODE_OK) {
      LOG_DEBUG("Queue received: %s\n", outstr);
    } else {
      LOG_DEBUG("Queue receive failed! Code: %d\n", ret);
    }

    delay_ms(50);  // Add a slight delay to avoid flooding logs
  }
}

// Main setup
int main(void) {
  log_init();

  StatusCode qret = queue_init(&s_my_queue);
  if (qret != STATUS_CODE_OK) {
    LOG_DEBUG("Queue init failed! Code: %d\n", qret);
    return 1;
  }

  tasks_init();

  tasks_init_task(task1, TASK_PRIORITY(2), NULL);
  tasks_init_task(task2, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Program start...\n");
  tasks_start();

  return 0;
}
