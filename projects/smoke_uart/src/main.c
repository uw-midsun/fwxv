#include <stdio.h>

#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "tasks.h"
#include "uart.h"

TASK(master_task, TASK_MIN_STACK_SIZE) {
  // while (true) {
  //   vTaskDelay(pdMS_TO_TICKS(1000));
  // }
}

int main() {
  log_init();
  gpio_init();
  gpio_it_init();
  interrupt_init();
  UartSettings smoke_settings = { .tx = { .port = GPIO_PORT_A, .pin = 10 },
                                  .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                  .baudrate = 9600 };
  uart_init(UART_PORT_2, &smoke_settings);

  tasks_init();

  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(master_task, TASK_PRIORITY(0), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
