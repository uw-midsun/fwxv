#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "delay.h"
#include "fsm.h"
#include "log.h"
#include "master_task.h"
#include "mcp2515.h"
#include "misc.h"
#include "motor_can.h"
#include "precharge_control.h"
#include "soft_timer.h"
#include "tasks.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};
static Mcp2515Storage s_mcp2515_storage = { 0 };
Mcp2515Settings mcp2515_settings = {  // place holder values
  .spi_port = SPI_PORT_1,
  .spi_settings = {
    0
  },
  .interrupt_pin = { GPIO_PORT_A, 0 },
  .can_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .loopback = true,
  },
};
PrechargeControlSettings precharge_settings = {
  // place holder values
  .precharge_control = { GPIO_PORT_A, 10 },
  .precharge_monitor = { GPIO_PORT_A, 9 },
  .precharge_monitor2 = { GPIO_PORT_A, 8 },
};

void pre_loop_init() {}

void run_fast_cycle() {
  run_can_rx_cycle();
  run_mcp2515_rx_cycle();
  wait_tasks(2);

  run_can_tx_cycle();
  run_mcp2515_tx_cycle();
  wait_tasks(2);
}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  can_init(&s_can_storage, &can_settings);
  mcp2515_init(&s_mcp2515_storage, &mcp2515_settings);
  precharge_control_init(&precharge_settings);
  init_motor_controller_can();
  LOG_DEBUG("Motor Controller Task\n");

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
