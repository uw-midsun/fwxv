#include <stdio.h>

#include "cc_hw_defs.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "seg_display.h"
#include "tasks.h"

SegDisplay all_displays = ALL_DISPLAYS;

TASK(seg_displays, TASK_STACK_512) {
  seg_displays_init(&all_displays);
  while (true) {
    seg_displays_set_int(&all_displays, 56, 123, 78);
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to TEST!");
  tasks_init_task(seg_displays, 1, NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
