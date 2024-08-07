#include <stdint.h>
#include <stdio.h>

#include "bms.h"
#include "can.h"
#include "can_board_ids.h"
#include "cell_sense.h"
#include "current_sense.h"
#include "fan.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "master_task.h"
#include "relays.h"
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

BmsStorage bms_storage;
uint32_t notification;

void pre_loop_init() {
  LOG_DEBUG("Welcome to BMS \n");
  fault_bps_init(&bms_storage.bps_storage);
  init_bms_relays(&kill_switch_mntr);
  current_sense_init(&bms_storage, &i2c_settings, FUEL_GAUGE_CYCLE_TIME_MS);
  cell_sense_init(&bms_storage);
  aux_sense_init(&bms_storage.aux_storage);
  bms_fan_init(&bms_storage);
}

void run_fast_cycle() {
  notify_get(&notification);
  if (notification & (1 << KILLSWITCH_IT)) {
    LOG_DEBUG("KILLSWITCH PRESSED\n");
    fault_bps_set(BMS_FAULT_KILLSWITCH);
  }
  run_can_rx_cycle();
  wait_tasks(1);
  run_can_tx_cycle();
  wait_tasks(1);
}

void run_medium_cycle() {
  // Afe Voltage Conversions
  cell_conversions();
  wait_tasks(1);
  cell_sense_run();
  delay_ms(10);
  current_sense_run();
  wait_tasks(1);
  aux_sense_run();
  bms_run_fan();
}

void run_slow_cycle() {
  // cell_discharge(&bms_storage.ltc_afe_storage);

  // if (fault_bps_get()) {
  //   LOG_DEBUG("FAULT_BITMASK: %d\n", fault_bps_get());
  //   delay_ms(3);
  // }
}  //

int main() {
  // Remove this in the future - Aryan
  set_master_cycle_time(200);
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
