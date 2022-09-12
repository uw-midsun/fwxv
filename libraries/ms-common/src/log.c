#include "log.h"

#define ITEM_SIZE MAX_LOG_SIZE
#define QUEUE_LENGTH 10

// Holds the output of snprintf
char log_buffer[MAX_LOG_SIZE];

// s_log_queue will hold the queue structure.
static StaticQueue_t s_log_queue;

// Array to hold items
static uint8_t s_log_array[QUEUE_LENGTH * ITEM_SIZE];

// Queue handle definition
QueueHandle_t g_log_queue = NULL;

void log_init(void) {
  g_log_queue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, s_log_array, &s_log_queue);
  if (log_task->context == NULL) {
    tasks_init_task(log_task, TASK_PRIORITY(tskIDLE_PRIORITY + 1), (int *)1);
  }
}

TASK(log_task, TASK_STACK_256) {
  // Buffer to hold the received message from QueueReceive
  static char rx_buffer[MAX_LOG_SIZE];

  // All tasks MUST loop forever and cannot return.
  while (true) {
    if (xQueueReceive(g_log_queue, rx_buffer, portMAX_DELAY)) {
      if (uxQueueSpacesAvailable(g_log_queue) <= 1) {
        printf("WARNING: Log queue is full\n");
      }
      printf("%s", rx_buffer);
    }
  }
}
