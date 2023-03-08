#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

#include "fsm.h"
#include "drive_fsm.h"
#include "power_fsm.h"
#include "delay.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

void run_fast_cycle() {}

void run_medium_cycle() {
  fsm_run_cycle(drive_fsm);
  wait_tasks(1);
  fsm_run_cycle(centre_console_power_fsm);
  wait_tasks(1);
  // delay_ms(1000);
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST! \n");

  init_drive_fsm();
  init_power_fsm();

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main? \n");
  return 0;
}
