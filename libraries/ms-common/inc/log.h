#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <cstdio>

#include "FreeRTOS.h"
#include "semaphore.h"
#include "task.h"
#include "tasks.h"
#include "uart.h"

// NOTE: Log debug cannot be used in core
// NOTE: Please use %lli instead of %li

DECLARE_TASK(log_task);

#define MAX_LOG_SIZE (size_t)200
#define LOG_TIMEOUT_MS 10

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

extern char g_log_buffer[MAX_LOG_SIZE];
extern Mutex s_log_mutex;
extern UartSettings log_uart_settings;

#define LOG_DEBUG(fmt, ...) LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG(LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

#ifdef MS_PLATFORM_X86
#define log_init() \
  { mutex_init(&s_log_mutex); }
#else
#define log_init()                           \
  {                                          \
    mutex_init(&s_log_mutex);                \
    uart_init(UARTPORT, &log_uart_settings); \
  }
#endif

#ifdef MS_PLATFORM_X86
#define LOG(level, fmt, ...) printf("[%u] %s:%u: " fmt, (level), __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(level, fmt, ...)                                                                 \
  do {                                                                                       \
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {                                 \
      if (mutex_lock(&s_log_mutex, LOG_TIMEOUT_MS) == STATUS_CODE_OK) {                      \
        size_t msg_size = (size_t)snprintf(g_log_buffer, MAX_LOG_SIZE, "\r[%u] %s:%u: " fmt, \
                                           (level), __FILE__, __LINE__, ##__VA_ARGS__);      \
        uart_tx(UARTPORT, (uint8_t *)g_log_buffer, &msg_size);                               \
      }                                                                                      \
      mutex_unlock(&s_log_mutex);                                                            \
    }                                                                                        \
  } while (0)
#endif
