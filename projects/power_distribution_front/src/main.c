#include <stdio.h>

#include "can.h"
#include "gpio.h"
#include "log.h"
#include "master_task.h"
#include "power_seq_fsm.h"
#include "tasks.h"

#define DEVICE_ID 0x04

#define DEVICE_ID 0x04

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to TEST!");

  can_init(&s_can_storage, &can_settings);
  init_power_seq();
  init_lights();
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
