#include <stdint.h>
#include <stdio.h>

#include "bms.h"
#include "can.h"
#include "can_board_ids.h"
#include "current_sense.h"
#include "fan.h"
#include "fault_bps.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ltc_afe_impl.h"
#include "master_task.h"
#include "max17261_fuel_gauge.h"
#include "pwm.h"
#include "relays_fsm.h"
#include "tasks.h"

#define FUEL_GAUGE_CYCLE_TIME_MS 100

const CurrentStorage current_storage;
const LtcAfeStorage ltc_afe_storage;

static CanStorage s_can_storage = { 0 };

// BmsStorage bms_storage = {
//   .current_storage = current_storage,
//   .ltc_afe_storage = ltc_afe_storage,
// };

static const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

void pre_loop_init() {
  LOG_DEBUG("Welcome to BMS \n");
  // init_bms_relays();
  bms_fan_init();
}

void run_fast_cycle() {
  bms_run_fan();
}

void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  // fsm_run_cycle(bms_relays);
  // wait_tasks(1);


  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  interrupt_init();
  gpio_init();
  can_init(&s_can_storage, &can_settings);

  LOG_DEBUG("Welcome to BMS!\n");
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
