#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_watchdog.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "master_task.h"
#include "new_can_getters.h"
#include "new_can_setters.h"
#include "tasks.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

void run_fast_cycle() {}

static uint8_t s_to_transmit;
void run_medium_cycle() {
  // We set the values we expect to receive, since all messages
  // send will be received by new_can in loopback mode
  run_can_rx_cycle();
  wait_tasks(1);
  LOG_DEBUG("Most Recent Received Data:\n");
  LOG_DEBUG("MSG1: %d\n", get_transmit_msg1_status());
  LOG_DEBUG("MSG2: %d\n", get_transmit_msg2_signal());

  s_to_transmit = (s_to_transmit + 1) % 2;
  set_transmit_msg1_status(s_to_transmit);
  set_transmit_msg2_signal(s_to_transmit);
  set_transmit_msg3_help(s_to_transmit);

  LOG_DEBUG("Sending messages with data: %d\n", s_to_transmit);
  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

int main() {
  gpio_init();
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to CAN!\n");
  can_init(&s_can_storage, &can_settings);
  can_add_filter_in(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1);

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
