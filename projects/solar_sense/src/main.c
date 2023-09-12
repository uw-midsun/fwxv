#include <stdio.h>

#include "adc.h"
#include "current_sense.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"

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

  LOG_DEBUG("Welcome to CAN!\n");
  can_init(&s_can_storage, &can_settings);
  can_add_filter_in(
      SYSTEM_CAN_MESSAGE_SOLAR_SENSE_CURRENT_SENSE);  // SYSTEM_CAN_MESSAGE_{BOARD_NAME}_{MSG_NAME}

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
