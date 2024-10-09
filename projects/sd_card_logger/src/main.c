#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"

void pre_loop_init() {}
void run_fast_cycle() {}

int main() {
  gpio_init();
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to SDLogger!\n");

  LOG_DEBUG("exiting main?\n");
  return 0;
}
