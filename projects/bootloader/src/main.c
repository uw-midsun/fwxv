#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "tasks.h"

int main() {
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  while (true) {
    // Listen for messages with bootloader+start ID (add to can.c)
    // Check which node it is for
    // Capture data
    // Erase memory and flash
  }

  LOG_DEBUG("exiting main?");
  return 0;
}
