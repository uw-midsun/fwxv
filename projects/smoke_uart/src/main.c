#include <stdio.h>

#include "gpio_it.h"
#include "interrupt.h"
#include "tasks.h"
#include "uart.h"

static GpioAddress gpios[] = { { .port = GPIO_PORT_A, .pin = 10 },
                               { .port = GPIO_PORT_A, .pin = 11 } };

TASK(master_task, TASK_MIN_STACK_SIZE) {
  // while (true) {
  //   vTaskDelay(pdMS_TO_TICKS(1000));
  // }
}

int main() {
  gpio_init();
  gpio_it_init();
  interrupt_init();

  gpio_init_pin(&gpios[0], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&gpios[1], GPIO_INPUT_PULL_DOWN, GPIO_STATE_HIGH);

  UartSettings smoke_settings = { .tx = gpios[0], .rx = gpios[1], .baudrate = 9600 };
  uart_init(UART_PORT_2, &smoke_settings);

  tasks_init();

  tasks_init_task(master_task, TASK_PRIORITY(0), NULL);

  tasks_start();

  return 0;
}
