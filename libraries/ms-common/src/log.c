#include "log.h"

// Allocating memory for extern variables in .h files
char g_log_buffer[MAX_LOG_SIZE];
Mutex s_log_mutex;