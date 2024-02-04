#include <stdint.h>
#include <stdio.h>

#include "bms.h"
#include "can.h"
#include "can_board_ids.h"
#include "current_sense.h"
#include "fault_bps.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "master_task.h"
#include "max17261_fuel_gauge.h"
#include "relays_fsm.h"
#include "tasks.h"

#define FUEL_GAUGE_CYCLE_TIME_MS 100

static CurrentStorage s_currentsense_storage;

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = BMS_PERIPH_I2C_SDA_PIN,
  .scl = BMS_PERIPH_I2C_SCL_PIN,
};

// const CurrentStorage current_storage = { 0 };

void pre_loop_init() {
  LOG_DEBUG("Welcome to BMS \n");
}

void run_fast_cycle() {
  fsm_run_cycle(relays);  // runs current sense cycle too
  wait_tasks(1);
}

void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  // run_current_sense_cycle();
  // wait_tasks(1);

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  can_init(&s_can_storage, &can_settings);
  i2c_init(BMS_PERIPH_I2C_PORT, &i2c_settings);

  init_relays();
  // current_sense_init(&s_currentsense_storage, &i2c_settings, FUEL_GAUGE_CYCLE_TIME_MS);

  LOG_DEBUG("Welcome to BMS!\n");
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
