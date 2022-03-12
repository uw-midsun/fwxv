#include "log.h"

#define ITEM_SIZE MAX_LOG_SIZE

// Holds the output of snprintf
char g_buffer[MAX_LOG_SIZE];

// s_log_queue will hold the queue structure.
static StaticQueue_t s_log_queue;

// Array to hold items
static uint8_t s_log_array[QUEUE_LENGTH * ITEM_SIZE];

// Holds the queue object (either make this into the task or put the variables in header so task can
// use it)
QueueHandle_t g_log_queue = NULL;

void log_init(void) {
  g_log_queue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, s_log_array, &s_log_queue);
  tasks_init_task(log_task, TASK_PRIORITY(MIN_TASK_PRIORITY), NULL);
}

TASK(log_task, TASK_STACK_256) {
  char rx_buffer[MAX_LOG_SIZE];

  // All tasks MUST loop forever and cannot return.
  while (true) {
    if (uxQueueSpacesAvailable(g_log_queue) == 0) {
      printf("Error: Log queue is full\n");
    }
    if (uxQueueMessagesWaiting(g_log_queue) != 0) {
      if (xQueueReceive(g_log_queue, (rx_buffer), (TickType_t)0)) {
        printf("%s", rx_buffer);
      }
    }
  }
}
