#include <stdbool.h>

#include "delay.h"
#include "log.h"
#include "queues.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"

#define LIST_SIZE 5
#define ITEM_SIZE sizeof(uint8_t)

#define QUEUE_LENGTH 5
#define BUF_SIZE (QUEUE_LENGTH * ITEM_SIZE)

static const uint8_t s_list[LIST_SIZE] = { 5, 6, 7, 8, 9 };
static uint8_t s_queue_buf[BUF_SIZE];
static Queue queue;

void setup_test(void) {
  log_init();

  queue.item_size = sizeof(uint8_t);
  queue.num_items = LIST_SIZE;
  queue.storage_buf = s_queue_buf;

  queue_init(&queue);
  TEST_ASSERT_EQUAL_UINT32(queue_get_num_items(&queue), QUEUE_LENGTH);
}

void teardown_test(void) {}

TASK(sendMessages, TASK_MIN_STACK_SIZE) {
  BaseType_t status;
  uint8_t index = 0;

  while (true) {
    do {
      status = queue_send(&queue, (void *)&s_list[index], 100);
      if (++index == QUEUE_LENGTH) {
        index = 0;
      }
    } while (status == STATUS_CODE_OK);
    delay_ms(2000);
  }
}

TASK(receiveMessages, TASK_MIN_STACK_SIZE) {
  BaseType_t status;
  uint8_t outstr = 0;
  uint8_t prev_outstr = 0;
  uint8_t index = 0;

  delay_ms(200);
  while (true) {
    prev_outstr = outstr;
    status = queue_receive(&queue, &outstr, 100);
    if (status != STATUS_CODE_OK) {
      // Outstrs are the same for previous and current iteration,
      // recieve failed
      TEST_ASSERT_EQUAL_UINT8(outstr, prev_outstr);
      delay_ms(100);
    } else {
      // Outstr is expected value
      TEST_ASSERT_EQUAL_UINT8(outstr, s_list[index]);
      if (++index == QUEUE_LENGTH) {
        index = 0;
      }
    }
  }
}

TASK(peekMessages, TASK_MIN_STACK_SIZE) {
  BaseType_t status;
  uint8_t outstr = 0;

  while (true) {
    status = queue_peek(&queue, &outstr, 200);
    if (status != STATUS_CODE_OK) {
    } else {
      // Peek runs immediately after the queue is filled so
      // assert first item sent
      TEST_ASSERT_EQUAL_UINT8(s_list[0], outstr);
      delay_ms(500);
    }
  }
}

TEST_IN_TASK
void test_running_task() {
  // Send
  tasks_init_task(sendMessages, TASK_PRIORITY(3), NULL);

  delay_ms(100);

  // Peek and receive
  tasks_init_task(peekMessages, TASK_PRIORITY(2), NULL);
  tasks_init_task(receiveMessages, TASK_PRIORITY(1), NULL);

  delay_ms(1000);
}
