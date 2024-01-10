#include <stdio.h>

#include "bts_load_switch.h"
#include "can.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "lights_fsm.h"
#include "log.h"
#include "master_task.h"
#include "output_current_sense.h"
#include "pin_defs.h"
#include "power_seq_fsm.h"
#include "tasks.h"

#define DEVICE_ID 0x06

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = DEVICE_ID,
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
  pd_output_init();
  pd_sense_init();
  adc_init();
}

void run_fast_cycle() {}
void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  fsm_run_cycle(power_seq);
  // fsm_run_cycle(lights);
  wait_tasks(1);

  adc_run();

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  interrupt_init();
  i2c_init(I2C_PORT_1, &i2c_settings);
  pca9555_gpio_init(I2C_PORT_1);
  can_init(&s_can_storage, &can_settings);
  set_master_cycle_time(250);  // Give it enough time to run an entire medium cycle
  set_medium_cycle_count(2);   // adjust medium cycle count to run once per 500ms
  init_power_seq();
  // init_lights();

  LOG_DEBUG("Welcome to PD!\n");
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
