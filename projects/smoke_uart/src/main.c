#include <stdio.h>

#include "delay.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"
#include "uart.h"
// const UartPort UART_PORT;

UartSettings smoke_settings = { .tx = { .port = GPIO_PORT_A, .pin = 9 },
                                .rx = { .port = GPIO_PORT_A, .pin = 10 },
                                .baudrate = 9600 };
TASK(master_task, TASK_MIN_STACK_SIZE) {
  uart_init(UART_PORT_2, &smoke_settings);
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
