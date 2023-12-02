#include <stdio.h>
#include <stdint.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

#include "gpio_it.h"
#include "current_sense.h"

void pre_loop_init() {}

void run_fast_cycle() {
  uint32_t alerts = 0;
  notify_get(&alerts);

  if (alerts & (1U << ALRT_GPIO_IT)) {
    // Check current
    // Check voltage
    // Check temps
  }
}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
