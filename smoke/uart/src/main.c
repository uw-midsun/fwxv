#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "string.h"
#include "tasks.h"
#include "uart.h"

#define MAX_CMD_LEN 50

static const char newline[] = "\n";

// This might cause issues because logs are initialized on the same port at a different baudrate,
// check this if there are issues
static UartSettings uart_settings = { .tx = { .port = GPIO_PORT_B, .pin = 6 },
                                      .rx = { .port = GPIO_PORT_B, .pin = 7 },
                                      .baudrate = 115200 };

static UartSettings uart2_settings = { .tx = { .port = GPIO_PORT_A, .pin = 2 },
                                      .rx = { .port = GPIO_PORT_A, .pin = 3 },
                                      .baudrate = 115200 };


TASK(master_task, TASK_STACK_512) {
  uint8_t data_buffer[MAX_CMD_LEN + 1] = { 0 };
  size_t idx = 0;
  uint8_t test = 0xAB;
  size_t test_size = 1;
  LOG_DEBUG("STARTING APPLICATION\n\r");
  while (true) {
    uart_tx(UART_PORT_2, &test, &test_size);
    size_t len = 1;
    uint8_t data = 0;
    StatusCode status = STATUS_CODE_EMPTY;
    while (status != STATUS_CODE_OK) {
      uart_tx(UART_PORT_2, &test, &test_size);
      len = 1;
      status = uart_rx(UART_PORT_1, &data, &len);
    }
    test = '\r';
    data_buffer[idx % MAX_CMD_LEN] = data;
    ++idx;

    if (data == '\r') {
      test=0xAB;
      LOG_DEBUG("%s\n", data_buffer);
      memset(data_buffer, 0, sizeof(data_buffer));
      idx = 0;
    }
  }
}

int main() {
  log_init();
  uart_init(UART_PORT_2, &uart2_settings);
  gpio_init();
  interrupt_init();

  tasks_init();

  tasks_init_task(master_task, TASK_PRIORITY(0), NULL);

  tasks_start();

  return 0;
}
