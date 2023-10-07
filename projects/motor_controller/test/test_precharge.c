#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_codegen.h"
#include "delay.h"
#include "fsm.h"
#include "gpio_it.h"
#include "interrupt.h"
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
Mcp2515Settings s_mcp2515_settings = {  // place holder values
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
  .precharge_control = { .port = GPIO_PORT_A, .pin = 10 },
  .precharge_monitor = { .port = GPIO_PORT_A, .pin = 9 },
  .precharge_monitor2 = { .port = GPIO_PORT_A, .pin = 8 },
};

CanQueue s_mcp2515_tx_queue;

// StatusCode TEST_MOCK(mcp2515_hw_transmit)(uint32_t id, bool extended, const uint64_t data,
//                                           size_t len) {
//   CanMessage message = {
//     .id = id,
//     .extended = extended,
//     .data = data,
//     .dlc = len,
//   } can_queue_push(mcp2515_tx_queue, &message);
// }

// static uint32_t prv_get_uint32(float f) {
//   union {
//     float f;
//     uint32_t u;
//   } fu = { .f = f };
//   return fu.u;
// }

bool initialized = false;

void setup_test(void) {
  if (initialized) return;
  initialized = true;

  interrupt_init();
  gpio_it_init();
  tasks_init();
  log_init();
  can_init(&s_can_storage, &can_settings);
  precharge_control_init(&precharge_settings);
}

void teardown_test(void) {}

void run_motor_controller_cycle() {
  run_can_rx_cycle();
  // run_mcp2515_rx_cycle();
  wait_tasks(1);
  run_can_tx_cycle();
  // run_mcp2515_tx_cycle();
  wait_tasks(1);

  LOG_DEBUG("completed cycle\n");
}

typedef union {
  uint8_t data[8];
  uint64_t data_64;
} Test;

// WE ONLY HAVE ONE MONITOR???
TEST_IN_TASK
void test_precharge(void) {
  // run_motor_controller_cycle();
  // TEST_ASSERT_EQUAL(MCI_PRECHARGE_DISCHARGED, g_tx_struct.mc_status_precharge_status);
}

TEST_IN_TASK
void test_precharge_2(void) {
  // run_motor_controller_cycle();

  // TEST_ASSERT_EQUAL(MCI_PRECHARGE_DISCHARGED, g_tx_struct.mc_status_precharge_status);

  // gpio_set_state(&precharge_settings.precharge_monitor, GPIO_STATE_HIGH);
  // gpio_it_trigger_interrupt(&precharge_settings.precharge_monitor);

  // TEST_ASSERT_EQUAL(MCI_PRECHARGE_INCONSISTENT, g_tx_struct.mc_status_precharge_status);
}
