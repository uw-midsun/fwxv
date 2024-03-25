#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_msg.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "master_task.h"
#include "pedal.h"
#include "tasks.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_PEDAL,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static PedalCalibBlob s_calib_blob = { 0 };

void pre_loop_init() {
  calib_init(&s_calib_blob, sizeof(s_calib_blob), false);
  pedal_init(&s_calib_blob);
}

void run_fast_cycle() {
  pedal_run();

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to PEDAL!\n");
  gpio_init();
  gpio_it_init();
  can_init(&s_can_storage, &can_settings);

  init_master_task();
  tasks_start();
  LOG_DEBUG("exiting main?\n");
  return 0;
}
