/*
  Project Code for FW 103

  Assignment: Create an ADC driver to interface with the ADS1115 Multi-Channel ADC IC.

  Requirements:
    - Implement the ADC driver functions (set config, select and read from a channel)
    - ADC task to periodically measure the voltage of channel 0
    - Overvoltage interrupt (configure the interrupt to be on channel 0 with thresholds of 0V - 1V)
*/

#include <stdio.h>

#include "log.h"
#include "tasks.h"

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to FW 103!\n");

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
