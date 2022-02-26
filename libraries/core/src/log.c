#include "log.h"

#include "FreeRTOS.h"
#include "queue.h"
#include <stdarg.h>
#include <stdio.h>
#include "tasks.h"

/* NOTES
* Top level view:
* user will call LOG_DEBUG(printf format) <- my implementation
* the arguments will get passed to string replacement
* string replacement will take those arguments then
* iterate through the variable parameters and <- use sprintf
* return a string
* this string is returned to the log_debug function
* function checks if the scheduler is off, if yes
* it passes the string to a base printf
* if not, then it creates a log_task, passing it the string
* this task then gets added to the queue and printfs when its turn comes
*/

// Holds the output of snprintf
static char s_buffer [MAX_LOG_SIZE];

// s_xQueueBuffer will hold the queue structure.
static StaticQueue_t s_xQueueBuffer;

// Array to hold items
static uint8_t *s_log_array[QUEUE_LENGTH * MAX_LOG_SIZE] = {0};

// Holds the queue object (either make this into the task or put the variables in header so task can use it)
static QueueHandle_t s_log_queue;

void log_init(void){
  tasks_init_task(log_task, TASK_PRIORITY(0));
}

void LOG(LogLevel level, const char* file, const char* line, const char * fmt, ... ){
  va_list args;
  va_start(args, fmt);

  int num_chars = snprintf(s_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, level, file, line, args);

  va_end(args);

  if(xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED || xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED){
    printf("%s", s_buffer);
    return ;
  }

  xQueueSendToBack( s_log_queue, ( void * ) s_buffer, ( TickType_t ) 0 );
  return ;
}

void LOG_C(LogLevel level, const char* file, const char* line, const char * fmt, ... ){
  // Adds message to the front of the queue
  va_list args;
  va_start(args, fmt);

  int num_chars = snprintf(s_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, level, file, line, args);

  va_end(args);

  if(xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED || xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED){
    printf("%s", s_buffer);
    return ;
  }

  xQueueSendToToFront( s_log_queue, ( void * ) s_buffer, ( TickType_t ) 0 );
  return ;
}

TASK(log_task, TASK_STACK_256) {
  // Do any setup.
  // |context| from tasks_init_task is passed as a void *context parameter.
  s_log_queue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, s_log_array, &s_xQueueBuffer);
  char Rx_buffer [MAX_LOG_SIZE];

  // All tasks MUST loop forever and cannot return.
  while (true) {
    if( s_log_queue != 0 ){
      if( xQueueReceive( s_log_queue, &( Rx_buffer ), ( TickType_t ) 0 ) ){
        printf("%s", Rx_buffer);
      }
    }
}
