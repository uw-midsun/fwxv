#include <stdbool.h>
#include <stdio.h>

#include "delay.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "tasks.h"

TASK(master_task, TASK_STACK_512) {
  int i = 0;
  while (true) {
    LOG_DEBUG("Hello World %d\n", i);
    ++i;
    // vTaskDelay(pdMS_TO_TICKS(100));
    delay_ms(100);
  }
}

int main(void) {
  log_init();
  tasks_init();

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Hello World...\n");

  tasks_start();

  return 0;
}
