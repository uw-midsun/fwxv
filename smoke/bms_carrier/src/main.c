#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "relays_fsm.h"
#include "spi.h"
#include "tasks.h"

TASK(smoke_bms_task, TASK_STACK_512) {
  init_bms_relays();
    close_relays();
  while (true) {
    LOG_DEBUG("CLOSING RELAYS!!!!\n");
    delay_ms(2000);
    // LOG_DEBUG("OPENING!!!! RELAYS!!!!\n");
    // open_relays();
    // delay_ms(2000);
  }
}

int main() {
  gpio_init();
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to BMS!!!!! TEST!\n");

  tasks_init_task(smoke_bms_task, TASK_PRIORITY(1), NULL);

  tasks_start();

  return 0;
}
