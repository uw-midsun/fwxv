#pragma once
// NOTE(Mitch): Since we are only using uart for logs on port 1, make others
// unavailable to save space
#define MULTI_PORT_UART 0
#define NUM_UART_PORTS 1

typedef enum {
  UART_PORT_1 = 0,
#ifdef MULTI_PORT_UART
  UART_PORT_2,
  UART_PORT_3,
  UART_PORT_4,
#endif
} UartPort;
