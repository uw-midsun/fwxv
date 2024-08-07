#include <stdio.h>

#include "bts_load_switch.h"
#include "can.h"
#include "can_board_ids.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "lights_fsm.h"
#include "log.h"
#include "master_task.h"
#include "output_current_sense.h"
#include "pd_fault.h"
#include "pin_defs.h"
#include "power_seq_fsm.h"
#include "tasks.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = PD_I2C_SDA,
  .scl = PD_I2C_SCL,
};

void pre_loop_init() {
  pca9555_gpio_init(I2C_PORT_1);
  pd_output_init();
  pd_sense_init();
  adc_init();
  pd_set_active_output_group(OUTPUT_GROUP_POWER_OFF);
}

void run_fast_cycle() {}
void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  adc_run();
  fsm_run_cycle(power_seq);
  fsm_run_cycle(lights);
  wait_tasks(2);

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
  i2c_init(I2C_PORT_1, &i2c_settings);
  init_power_seq();
  init_lights();
  init_bps_fault();

  LOG_DEBUG("Welcome to PD!\n");
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
