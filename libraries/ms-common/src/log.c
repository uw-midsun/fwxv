#include "log.h"

// Allocating memory for extern variables in .h files
char g_log_buffer[MAX_LOG_SIZE];
Mutex s_log_mutex;

UartSettings log_uart_settings = { .tx = { GPIO_PORT_A, TX_PIN },  // tx pin
                                   .rx = { GPIO_PORT_A, RX_PIN },  // rx pin
                                   .baudrate = 115200 };
