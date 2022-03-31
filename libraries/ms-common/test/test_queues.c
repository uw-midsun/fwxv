#include <stdbool.h>

#include "delay.h"
#include "log.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "queues.h"
#include "unity.h"

#define LIST_SIZE 5
#define ITEM_SIZE sizeof(uint8_t)

#define QUEUE_LENGTH 5
#define BUF_SIZE (QUEUE_LENGTH * ITEM_SIZE)

static const uint8_t s_list[LIST_SIZE] = {
	5, 6, 7, 8, 9
};

static StaticQueue_t s_queue;
static uint8_t s_queue_buf[BUF_SIZE];
static QueueHandle_t s_queue_handle;

static Queue queue;

static bool s_task_started;

void setup_test(void) {
  s_task_started = false;

  queue.handle = s_queue_handle;
  queue.queue = s_queue;
  queue.item_size = sizeof(uint8_t);
  queue.num_items = LIST_SIZE;
  queue.storage_buf = s_queue_buf;

  BaseType_t status;
  status = queue_init(&queue);
  if(status == STATUS_CODE_OK)
    LOG_DEBUG("Q created with %d item capacity\n", queue_get_num_items(&queue));
}

void teardown_test(void) {}

TASK(sendMessages, TASK_STACK_512) {
  s_task_started = true;
  BaseType_t status;
  uint8_t index = 0;
  volatile TickType_t tick_count;

  while (true) {
    do {
      status = queue_send(&queue, (void *) &s_list[index], 100);
      tick_count = xTaskGetTickCount();
      if (status==STATUS_CODE_OK)
        LOG_DEBUG("Sent int: %d at tick %lu\n", s_list[index], tick_count);
      if (++index == 5) {
        index = 0;
      }
    } while(status == STATUS_CODE_OK);
    LOG_DEBUG("Delayed until tick %lu and failed to send\n", tick_count);
    delay_ms(2000);
  }
}

TASK(receiveMessages, TASK_STACK_512) {
    BaseType_t status;
    const uint8_t outstr = 0;
    volatile TickType_t tick_count;

    delay_ms(200);
    while (true) {
      status = queue_receive(&queue, &outstr, 100);
      tick_count = xTaskGetTickCount();
      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("Error reading from queue at tick %lu\n", tick_count);
        delay_ms(100);
        continue;
      }
      LOG_DEBUG("Received int: %d at tick %lu\n", outstr, tick_count);
    }
}

TASK(peekMessages, TASK_STACK_512) {
    BaseType_t status;
    const uint8_t outstr = 0;
    volatile TickType_t tick_count;

    while(true) {
      status = queue_peek(&queue, &outstr, 200);
      tick_count = xTaskGetTickCount();
      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("Failed to peek at tick %lu\n", tick_count);
      } else {
        LOG_DEBUG("Peeked item %d at %lu\n", outstr, tick_count);
        delay_ms(500);
      }
    }
}

TASK_TEST(test_running_task, TASK_STACK_512) {
  // Send
  tasks_init_task(sendMessages, TASK_PRIORITY(3), NULL);
  
  TEST_ASSERT_FALSE(s_task_started);
  delay_ms(100);
  TEST_ASSERT_TRUE(s_task_started);

  // Queue should be full
  bool status;
  uint8_t num = 1;
  if(queue_send(&queue, (void *) &num, 0) != STATUS_CODE_OK)
    status = true;
  else
    status = false;
  TEST_ASSERT_TRUE(status);

  // Peek and receive
  tasks_init_task(peekMessages, TASK_PRIORITY(2), NULL);
  tasks_init_task(receiveMessages, TASK_PRIORITY(1), NULL);

  delay_ms(1000);

  // Queue is empty
  const uint8_t outstr = 0;
  if(queue_peek(&queue, &outstr, 0) != STATUS_CODE_OK)
    status = true;
  else
    status = false;
  
  TEST_ASSERT_TRUE(status);
}
