#include "cli.h"

#include <stdio.h>

#include "ctype.h"
#include "interrupt.h"
#include "log.h"
#include "string.h"
#include "tasks.h"
#include "uart.h"


TASK(cli_task, TASK_STACK_512) {
  cli_init();
  while (true) {
    cli_run();
  }
}

int main() {
  tasks_init();
  log_init();

  tasks_init_task(cli_task, TASK_PRIORITY(2), NULL);
  tasks_start();

  return 0;
}
