#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "delay.h"
#include "fsm.h"
#include "gpio_it.h"
#include "interrupt.h"
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
static Mcp2515Settings s_mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .spi_settings = {
    .baudrate = 10000000,  // 10 Mhz
    .mode = SPI_MODE_0,
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_B, 12 },
  },
  .interrupt_pin = { GPIO_PORT_A, 8 },
  .RX0BF = { GPIO_PORT_B, 10 },
  .RX1BF = { GPIO_PORT_B, 11 },
  .can_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .loopback = true,
  },
};
static PrechargeControlSettings s_precharge_settings = {
  .precharge_control = { GPIO_PORT_A, 9 },
  .precharge_monitor = { GPIO_PORT_A, 10 },
};

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
  interrupt_init();
  gpio_it_init();
  can_init(&s_can_storage, &can_settings);
  mcp2515_init(&s_mcp2515_storage, &s_mcp2515_settings);
  precharge_control_init(&s_precharge_settings);
  init_motor_controller_can();
  LOG_DEBUG("Motor Controller Task\n");

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
