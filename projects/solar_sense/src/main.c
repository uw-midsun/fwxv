#include <stdio.h>

// #include "can.h"
// #include "can_board_ids.h"
#include "i2c.h"
#include "log.h"
#include "master_task.h"
#include "mppt.h"
#include "tasks.h"

// static CanStorage s_can_storage = { 0 };
// static const CanSettings can_settings = {
//   .device_id = 0x1,
//   .bitrate = CAN_HW_BITRATE_500KBPS,
//   .tx = { GPIO_PORT_A, 12 },
//   .rx = { GPIO_PORT_A, 11 },
//   .loopback = true,
// };

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  init_master_task();
  // can_init(&s_can_storage, &can_settings);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
