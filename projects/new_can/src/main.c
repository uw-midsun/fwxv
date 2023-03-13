#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "log.h"
#include "new_can_setters.h"
#include "tasks.h"
#include "master_task.h"
#include "delay.h"
#include "can_watchdog.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

void run_fast_cycle() {
}

void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to CAN!\n");
  can_init(&s_can_storage, &can_settings);
  can_watchdogs_init();
  can_add_filter_in(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1);
  can_add_watchdog(0, 1);

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
