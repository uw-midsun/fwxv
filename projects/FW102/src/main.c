#include <stdio.h>

#include "log.h"
#include "tasks.h"


int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
