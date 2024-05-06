#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "tasks.h"
#include "uart.h"

static const char test[] = "test uart\n";

// This might cause issues because logs are initialized on the same port at a different baudrate,
// check this if there are issues
static UartSettings uart_settings = { .tx = { .port = GPIO_PORT_B, .pin = 6 },
                                      .rx = { .port = GPIO_PORT_B, .pin = 7 },
                                      .baudrate = 9600 };

TASK(master_task, TASK_STACK_512) {
  const size_t len = 15;
  uint8_t data_buffer[50];

  while (true) {
    StatusCode status;

    delay_ms(5000);

    status = uart_rx(UART_PORT_1, data_buffer, &len);
    if (status == STATUS_CODE_OK) {
      uart_tx(UART_PORT_1, data_buffer, &len);
    } else {
      LOG_DEBUG("Read failed: status code %d\n", status);
    }
    delay_ms(1000);
  }
}

int main() {
  uart_init(UART_PORT_1, &uart_settings);
  gpio_init();
  interrupt_init();

  tasks_init();

  tasks_init_task(master_task, TASK_PRIORITY(0), NULL);

  tasks_start();

  return 0;
}
