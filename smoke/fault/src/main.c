#include <stdio.h>
#include <stdint.h>

#include "fault.h"

#include "log.h"
#include "tasks.h"
#include "master_task.h"
#include "delay.h"
// #include "centre_console_getters.h" <- not getting included for some reason

void run_fast_cycle()
{

}

void run_medium_cycle()
{

}

void run_slow_cycle()
{

}

void fault_handler() {
   LOG_DEBUG("IN FAULT HANDLER\n");
   non_blocking_delay_ms(100);
}

TASK(master_task, TASK_STACK_512) {
  while (true) {
   run_fast_cycle();
   delay_ms(50);
  }
}

int main() {
   tasks_init();
   log_init();
   LOG_DEBUG("Welcome to TEST!");

   FaultStorage storage = {
      // Watch critical faults
      // If any of these are non-zero, the fault task takes over the ECU and
      // will require a power cycle
      .faults = {
         1, // get_solar_fault_5_mppts_fault(),
         2, // get_solar_fault_6_mppts_fault()
      },
      .num_faults = 2,

      // Watch critical messages
      // If any of these are not received in FAULT_CYCLES, the fault task takes over the ECU and
      // will require a power cycle
      .watching = {
         0, // get_received_solar_fault_5_mppts(),
         0, // get_received_solar_fault_6_mppts(),
         0, // get_received_battery_relay_state()
      },

      .num_watching = 3,
      .handler = fault_handler
   };

   fault_init(&storage);

   tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

   tasks_start();

   LOG_DEBUG("exiting main?");
   return 0;
}

