#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "tasks.h"

// NOTE: Log debug cannot be used in core

DECLARE_TASK(log_task);

#define QUEUE_LENGTH 32
#define MAX_LOG_SIZE (size_t)200

#ifndef IN_ORDER_LOGS
#define IN_ORDER_LOGS 0
#endif

// Global queue handle used in all log calls
extern QueueHandle_t g_log_queue;

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_CRITICAL,
  NUM_LOG_LEVELS,
} LogLevel;

void log_init(void);

#define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG_C(LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

#define LOG(level, fmt, ...)                                                            \
  do {                                                                                  \
    char log_buffer[MAX_LOG_SIZE];                                                      \
    snprintf(log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, \
             ##__VA_ARGS__);                                                            \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                            \
      printf("%s", log_buffer);                                                         \
    } else {                                                                            \
      if (g_log_queue == NULL) {                                                        \
        printf("Error: Logs init not initialized \n");                                  \
        vTaskEndScheduler();                                                            \
      }                                                                                 \
      xQueueSendToBack(g_log_queue, log_buffer, 0);                                     \
    }                                                                                   \
  } while (0)

#define LOG_C(level, fmt, ...)                                                          \
  do {                                                                                  \
    char log_buffer[MAX_LOG_SIZE];                                                      \
    snprintf(log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, \
             ##__VA_ARGS__);                                                            \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                            \
      printf("%s", log_buffer);                                                         \
    } else {                                                                            \
      if (g_log_queue == NULL) {                                                        \
        printf("Error: Logs init not initialized \n");                                  \
        vTaskEndScheduler();                                                            \
      }                                                                                 \
      if (IN_ORDER_LOGS) {                                                              \
        xQueueSendToBack(g_log_queue, log_buffer, 0);                                   \
      } else {                                                                          \
        xQueueSendToFront(g_log_queue, log_buffer, 0);                                  \
      }                                                                                 \
    }                                                                                   \
  } while (0)
