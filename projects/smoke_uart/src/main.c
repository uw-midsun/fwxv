#include <stdio.h>

#include "interrupt.h"
#include "log.h"
#include "tasks.h"
#include "uart.h"

#define SHOULD_TRANSMIT true
#define SHOULD_READ true

#define UART_PORT_SEND UART_PORT_1
#define UART_PORT_RECEIVE UART_PORT_2

static UartSettings uart1_settings = { .tx = { .port = GPIO_PORT_A, .pin = 10 },
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .baudrate = 9600 };

static UartSettings uart2_settings = { .tx = { .port = GPIO_PORT_B, .pin = 4 },
                                       .rx = { .port = GPIO_PORT_B, .pin = 5 },
                                       .baudrate = 9600 };
TASK(master_task, TASK_MIN_STACK_SIZE) {
  uart_init(UART_PORT_SEND, &uart1_settings);
  uart_init(UART_PORT_RECEIVE, &uart2_settings);

  uint8_t data_received = '1';
  uint8_t data_sent = '1';
  size_t len = sizeof(uint8_t);

  while (true) {
    StatusCode status;
    if (SHOULD_TRANSMIT) {
      status = uart_tx(UART_PORT_SEND, &data_sent, &len);
      if (status == STATUS_CODE_OK) {
        LOG_DEBUG("Successfully transmitted %u\n", data_sent);
      } else {
        LOG_DEBUG("Write failed: status code %d\n", status);
      }
    }
    if (SHOULD_READ) {
      status = uart_rex(UART_PORT_RECEIVE, &data_received, &len);
      if (status == STATUS_CODE_OK) {
        LOG_DEBUG("Successfully received %u\n", data_received);
      } else {
        LOG_DEBUG("Write failed: status code %d\n", status);
      }
    }
  }
}

int main() {
  log_init();
  gpio_init();
  interrupt_init();

  tasks_init();

  tasks_init_task(master_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}
