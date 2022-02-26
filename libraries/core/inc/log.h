#pragma once

#include <stdio.h>
#include <stdlib.h>

DECLARE_TASK(log_task);

// This definition is tentative
#define QUEUE_LENGTH 32
#define MAX_LOG_SIZE 100

#define MIN_TASK_PRIORITY 0

#define LOG_DEBUG(fmt, ... ) LOG(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ... ) LOG(LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ... ) LOG_C(LOG_LEVEL_CRITICAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_CRITICAL,
  NUM_LOG_LEVELS,
} LogLevel;

void log_init(void);

void LOG(LogLevel level, const char* file, const char* line, const char * fmt, ... ){

void LOG_C(LogLevel level, const char* file, const char* line, const char * fmt, ... ){
