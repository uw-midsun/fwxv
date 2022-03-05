#include "log.h"

#define ITEM_SIZE sizeof(g_buffer)

// Holds the output of snprintf
char g_buffer [MAX_LOG_SIZE];

// s_xQueueBuffer will hold the queue structure.
static StaticQueue_t s_xQueueBuffer;

// Array to hold items
static uint8_t s_log_array[QUEUE_LENGTH * ITEM_SIZE] = {0};

// Holds the queue object (either make this into the task or put the variables in header so task can use it)
QueueHandle_t g_log_queue;

void log_init(void){
  g_log_queue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, s_log_array, &s_xQueueBuffer);
  tasks_init_task(log_task, TASK_PRIORITY(MIN_TASK_PRIORITY), NULL);
}

TASK(log_task, TASK_STACK_256) {
  char Rx_buffer [MAX_LOG_SIZE];

  // All tasks MUST loop forever and cannot return.
  while (true) {
    if( uxQueueMessagesWaiting(g_log_queue) != 0 )
      if( xQueueReceive( g_log_queue, &( Rx_buffer ), ( TickType_t ) 0 ) )
        printf("%s", Rx_buffer);
  }
}
