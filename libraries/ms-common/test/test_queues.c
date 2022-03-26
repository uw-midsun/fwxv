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
	1, 2, 3, 4, 5
};

static StaticQueue_t s_queue;
static uint8_t s_queue_buf[BUF_SIZE];
static QueueHandle_t s_queue_handle;

static Queue* queue;

static bool s_task_started;

void setup_test(void) {
  s_task_started = false;
  Queue queue1 = {
    .handle = s_queue_handle,
    .queue = s_queue,
    .item_size = sizeof(uint8_t),
    .num_items = LIST_SIZE,
    .storage_buf = s_queue_buf
  };

  queue = &queue1;

  BaseType_t status;
  status = queue_init(queue);
  if(status == STATUS_CODE_OK) LOG_DEBUG("Q created\n");
}

void teardown_test(void) {}

TASK(sendMessages, TASK_STACK_512) {
  s_task_started = true;
  BaseType_t status;
  uint8_t index = 0;

  while (true) {
    delay_ms(100);
    status = queue_send(queue, (void *) &s_list[index], 0);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Error writing to queue %d\n", index);
      continue;
    }
    if (++index == 5) {
      index = 0;
    }
  }
}

TASK(printMessages, TASK_STACK_512) {
    BaseType_t status;
    const uint8_t out;
    while (true) {
      delay_ms(100);
      status = queue_receive(queue, out, 0);
      if (status != STATUS_CODE_OK) {
        LOG_DEBUG("Error reading from queue\n");
        continue;
      }
      LOG_DEBUG("Received string: %s in task 2\n", out);
    }
}

TASK_TEST(test_running_task, TASK_STACK_512) {
  // Start the task: note no need to call tasks_start() because we're inside a test task and
  // FreeRTOS is already running.
  tasks_init_task(sendMessages, TASK_PRIORITY(1), NULL);
  // tasks_init_task(printMessages, TASK_PRIORITY(1), NULL);

  // The task doesn't start immediately because the test task has the highest priority.
  TEST_ASSERT_FALSE(s_task_started);

  // To let it run, use a delay.
  delay_ms(1000);

  // The task should have run.
  TEST_ASSERT_TRUE(s_task_started);
}