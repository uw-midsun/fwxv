#include "log.h"

#include "FreeRTOS.h"
#include "queue.h"
#include <stdarg.h>
#include <stdio.h>
#include "task.h"

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

// This definition is tentative
#define QUEUE_LENGTH 32
#define MAX_LOG_SIZE 100

static char buffer [MAX_LOG_SIZE];

// xQueueBuffer will hold the queue structure.
StaticQueue_t xQueueBuffer;

// Array to hold items
uint8_t *log_array[QUEUE_LENGTH * MAX_LOG_SIZE] = {0};

// Holds the queue object (either make this into the task or put the variables in header so task can use it)
QueueHandle_t log_queue;

void log_init(void){

  log_queue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, log_array, &xQueueBuffer);
  /* use functions:
      xQueueSendToToFront
      xQueueSendToBack
      xQueueReceive
      maybe : uxQueueMessagesWaiting
      vQueueDelete - this is for deallocating the entire queue, used when shutting down the log task
  */
}

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_CRITICAL,
  NUM_LOG_LEVELS,
} LogLevel;

#define LOG_DEBUG(fmt, ... ) LOG(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ... ) LOG(LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ... ) LOG_C(LOG_LEVEL_CRITICAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


void LOG(LogLevel level, const char* file, const char* line, const char * fmt, ... ){
  va_list args;
  va_start(args, fmt);

  int num_chars = snprintf(buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, level, file, line, args);

  va_end(args);

  if(xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED || xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED){
    printf("%s", buffer);
    return ;
  }

  xQueueSendToBack( log_queue, ( void * ) buffer, ( TickType_t ) 0 );
  return ;
}

void LOG_C(LogLevel level, const char* file, const char* line, const char * fmt, ... ){
  // Adds message to the front of the queue
  // Allow blocking of the queue and freeRTOS
  va_list args;
  va_start(args, fmt);

  int num_chars = snprintf(buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, level, file, line, args);

  va_end(args);

  if(xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED || xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED){
    printf("%s", buffer);
    return ;
  }

  xQueueSendToToFront( log_queue, ( void * ) buffer, ( TickType_t ) 0 );
  return ;
}
