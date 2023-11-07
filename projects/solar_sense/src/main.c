#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "solar_sense_getters.h"
#include "tasks.h"
#include "temp_sense.h"

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {}

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

int main() {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to TEST!");
  adc_init();
  can_init(&s_can_storage, &can_settings);
  temp_sense_adc_init();

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
