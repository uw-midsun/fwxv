#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "retarget_cfg.h"
#include "task.h"
#include "tasks.h"
#include "uart.h"

// NOTE: Log debug cannot be used in core
// NOTE: Please use %lli instead of %li

DECLARE_TASK(log_task);

#define MAX_LOG_SIZE (size_t)200

#define UARTPORT UART_PORT_1
#define TX_PIN RETARGET_CFG_UART_GPIO_TX
#define RX_PIN RETARGET_CFG_UART_GPIO_RX

#ifndef IN_ORDER_LOGS
#define IN_ORDER_LOGS 1
#endif

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

#ifdef x86
#define LOG(level, fmt, ...)                                                                  \
  do {                                                                                        \
    char log_buffer[MAX_LOG_SIZE];                                                            \
    size_t msg_size = (size_t)snprintf(log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), \
                                       __FILE__, __LINE__, ##__VA_ARGS__);                    \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                                  \
      printf("%s", log_buffer);                                                               \
    } else {                                                                                  \
      uart_tx(UARTPORT, (uint8_t *)log_buffer, &msg_size);                                    \
    }                                                                                         \
  } while (0)
#else
#define LOG(level, fmt, ...)                                                                  \
  do {                                                                                        \
    char log_buffer[MAX_LOG_SIZE];                                                            \
    size_t msg_size = (size_t)snprintf(log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), \
                                      __FILE__, __LINE__, ##__VA_ARGS__);                    \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                                  \
      printf("%s", log_buffer);                                                               \
    }                                                                                        \
  } while (0)
#endif

#ifdef x86
#define LOG_C(level, fmt, ...)                                                                \
  do {                                                                                        \
    char log_buffer[MAX_LOG_SIZE];                                                            \
    size_t msg_size = (size_t)snprintf(log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), \
                                       __FILE__, __LINE__, ##__VA_ARGS__);                    \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                                  \
      printf("%s", log_buffer);                                                               \
    } else {                                                                                  \
      if (IN_ORDER_LOGS) {                                                                    \
        uart_tx(UARTPORT, (uint8_t *)log_buffer, &msg_size);                                  \
      }                                                                                       \
    }                                                                                         \
  } while (0)
#else
#define LOG_C(level, fmt, ...)                                                                \
  do {                                                                                        \
    char log_buffer[MAX_LOG_SIZE];                                                            \
    size_t msg_size = (size_t)snprintf(log_buffer, MAX_LOG_SIZE, "[%u] %s:%u: " fmt, (level), \
                                       __FILE__, __LINE__, ##__VA_ARGS__);                    \
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {                                  \
      printf("%s", log_buffer);                                                               \
    }                                                                                        \
  } while (0)
#endif
