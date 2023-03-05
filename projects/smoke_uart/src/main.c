#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "tasks.h"
#include "uart.h"

static const char test[] = "test uart\n";

static UartSettings uart_settings = { .tx = { .port = GPIO_PORT_A, .pin = 10 },
                                      .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                      .baudrate = 9600 };

TASK(master_task, TASK_MIN_STACK_SIZE) {
  uart_init(UART_PORT_1, &uart_settings);

  size_t len = sizeof(uint8_t);

  while (true) {
    StatusCode status;
    status = uart_tx(UART_PORT_1, test, &len);
    if (status == STATUS_CODE_OK) {
      LOG_DEBUG("Successfully transmitted %s\n", test);
    } else {
      LOG_DEBUG("Write failed: status code %d\n", status);
    }
    delay_ms(1000);
  }
}

int main() {
  log_init();
  gpio_init();
  interrupt_init();

  tasks_init();

  tasks_init_task(master_task, TASK_PRIORITY(0), NULL);

  tasks_start();

  return 0;
}
