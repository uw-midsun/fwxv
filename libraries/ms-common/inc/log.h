#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "semaphore.h"
#include "task.h"
#include "tasks.h"
#include "uart.h"

// NOTE: Log debug cannot be used in core
// NOTE: Please use %lli instead of %li

DECLARE_TASK(log_task);

#define MAX_LOG_SIZE (size_t)200

#define UARTPORT UART_PORT_1
#define TX_PIN 6
#define RX_PIN 7

#ifndef IN_ORDER_LOGS
#define IN_ORDER_LOGS 1
#endif

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_CRITICAL,
  NUM_LOG_LEVELS,
} LogLevel;

static char log_buffer[MAX_LOG_SIZE];
static Mutex s_log_mutex;

#define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG(LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

#define log_init()                        \
  do {                                    \
    extern char log_buffer[MAX_LOG_SIZE]; \
    extern Mutex s_log_mutex;             \
    mutex_init(&s_log_mutex);             \
  } while (0)

#ifdef MS_PLATFORM_X86
#define LOG(level, fmt, ...)                                                \
  do {                                                                      \
    printf("[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__); \
  } while (0)
#else
#define LOG(level, fmt, ...)                                                                  \
  do {                                                                                        \
    mutex_lock(&s_log_mutex, 0);                                                              \
    size_t msg_size = (size_t)snprintf(log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), \
                                       __FILE__, __LINE__, ##__VA_ARGS__);                    \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                                  \
      printf("%s", log_buffer);                                                               \
    } else {                                                                                  \
      uart_tx(UARTPORT, (uint8_t *)log_buffer, &msg_size);                                    \
    }                                                                                         \
    mutex_unlock(&s_log_mutex);                                                               \
  } while (0)
#endif
