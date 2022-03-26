#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "tasks.h"

DECLARE_TASK(log_task);

// This definition is tentative
#define QUEUE_LENGTH 32
#define MAX_LOG_SIZE (size_t)200

#ifndef IN_ORDER_LOGS
#define IN_ORDER_LOGS 0
#endif

#define MIN_TASK_PRIORITY 1

extern QueueHandle_t g_log_queue;
extern char g_log_buffer[MAX_LOG_SIZE];

#define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG_C(LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

#define LOG(level, fmt, ...)                                                              \
  do {                                                                                    \
    snprintf(g_log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, \
             ##__VA_ARGS__);                                                              \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                              \
      printf("%s", g_log_buffer);                                                         \
    } else {                                                                              \
      if (g_log_queue == NULL) {                                                          \
        printf("Error: Logs not initialized message \n");                                 \
        vTaskEndScheduler();                                                              \
      }                                                                                   \
      xQueueSendToBack(g_log_queue, g_log_buffer, 0);                                     \
    }                                                                                     \
  } while (0)

#define LOG_C(level, fmt, ...)                                                            \
  do {                                                                                    \
    snprintf(g_log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, \
             ##__VA_ARGS__);                                                              \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                              \
      printf("%s", g_log_buffer);                                                         \
    } else {                                                                              \
      if (g_log_queue == NULL) {                                                          \
        printf("Error: Logs init not initialized \n");                                    \
        vTaskEndScheduler();                                                              \
      }                                                                                   \
      if (IN_ORDER_LOGS) {                                                                \
        xQueueSendToBack(g_log_queue, g_log_buffer, 0);                                   \
      } else {                                                                            \
        xQueueSendToFront(g_log_queue, g_log_buffer, 0);                                  \
      }                                                                                   \
    }                                                                                     \
  } while (0)

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_CRITICAL,
  NUM_LOG_LEVELS,
} LogLevel;

void log_init(void);
