#include <stdint.h>
#include <stdio.h>

#include "bms.h"
#include "can.h"
#include "can_board_ids.h"
#include "cell_sense.h"
#include "current_sense.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "master_task.h"
#include "relays.h"
#include "state_of_charge.h"
#include "tasks.h"

#define FUEL_GAUGE_CYCLE_TIME_MS 100

static CanStorage s_can_storage = { 0 };
static GpioAddress kill_switch_mntr = { .port = GPIO_PORT_A, .pin = 15 };

static const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static const I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = BMS_PERIPH_I2C_SDA_PIN,
  .scl = BMS_PERIPH_I2C_SCL_PIN,
};

BmsStorage bms_storage = { .config = { .series_count = NUM_SERIES_CELLS,
                                       .parallel_count = NUM_PARALLEL_CELLS,
                                       .pack_capacity = PACK_CAPACITY_MAH } };

uint32_t notification;

void pre_loop_init() {
  LOG_DEBUG("Welcome to BMS \n");
  fault_bps_init(&bms_storage);
  init_bms_relays(&kill_switch_mntr);
  current_sense_init(&bms_storage, &i2c_settings, FUEL_GAUGE_CYCLE_TIME_MS);
  cell_sense_init(&bms_storage);
  state_of_charge_init(&bms_storage);
}

void run_fast_cycle() {}

void run_medium_cycle() {
  // run_can_rx_cycle();
  // wait_tasks(1);
  current_sense_run();
  wait_tasks(1);
  // update_state_of_chrage();
  log_cell_sense();
  // run_can_tx_cycle();
  // wait_tasks(1);
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  gpio_it_init();
  can_init(&s_can_storage, &can_settings);

  LOG_DEBUG("Welcome to BMS!\n");
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
