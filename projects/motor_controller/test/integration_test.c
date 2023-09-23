#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "delay.h"
#include "fsm.h"
#include "log.h"
#include "mcp2515.h"
#include "misc.h"
#include "motor_can.h"
#include "precharge_control.h"
#include "soft_timer.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
  .mode = CAN_CONTINUOUS,
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
    .mode = CAN_CONTINUOUS,
  },
};
PrechargeControlSettings precharge_settings = {
  // place holder values
  .precharge_control = { GPIO_PORT_A, 10 },
  .precharge_monitor = { GPIO_PORT_A, 9 },
  .precharge_monitor2 = { GPIO_PORT_A, 8 },
};

bool initialized = false;

void setup_test(void) {
  if (initialized) return;
  initialized = true;
  tasks_init();
  log_init();
  StatusCode status;
  status = can_init(&s_can_storage, &can_settings);
  LOG_DEBUG("%d\n", status);
  mcp2515_init(&s_mcp2515_storage, &mcp2515_settings);
  precharge_control_init(&precharge_settings);
  init_motor_controller_can();
}

void teardown_test(void) {}

void run_motor_controller_cycle() {
  run_can_rx_cycle();
  run_mcp2515_rx_cycle();
  wait_tasks(2);
  //   run_can_tx_cycle();
  //   run_mcp2515_tx_cycle();
  //   wait_tasks(2);
}

void test_motor_controller(void) {
  CanMessage msg = {};
  can_queue_push(&s_can_storage.rx_queue, &msg);
  run_motor_controller_cycle();
}
