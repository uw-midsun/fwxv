#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

<<<<<<< HEAD
int main() {
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  while (true) {
    // Listen for messages with bootloader+start ID (add to can.c)
    // Check which node it is for
    // Capture data
    // Erase memory and flash
  }
=======
void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  init_master_task();

  tasks_start();
>>>>>>> bootloader datagram setup

  LOG_DEBUG("exiting main?");
  return 0;
}
