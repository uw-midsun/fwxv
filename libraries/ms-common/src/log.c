#include "log.h"

#define ITEM_SIZE MAX_LOG_SIZE
#define QUEUE_LENGTH 10

// Holds the output of snprintf
char log_buffer[MAX_LOG_SIZE];

static UartSettings settings = {
  .tx = { TX_PIN }, .rx = { RX_PIN }, .alt_fn = 1, .baudrate = 9600
};

void log_init(void) {
  #ifdef x86
  uart_init(UARTPORT, &settings);
  #endif
  tasks_init_task(log_task, TASK_PRIORITY(tskIDLE_PRIORITY + 1), NULL);
}

TASK(log_task, TASK_STACK_256) {
  // Buffer to hold the received message from QueueReceive
  static uint8_t rx_buffer[MAX_LOG_SIZE];
  static StatusCode status;
  static size_t rx_size;

  // All tasks MUST loop forever and cannot return.
  while (true) {
    #ifdef x86
    status = uart_rx(UARTPORT, rx_buffer, &rx_size);
    if (status != STATUS_CODE_INVALID_ARGS) {
      if (status == STATUS_CODE_INCOMPLETE) {
        printf("WARNING: Log queue is full\n");
      }
      printf("%s", rx_buffer);
    }
    #endif
  }
}
