#include <stdio.h>
#include "operation_listener.h"
#include "can.h"
#include "can_board_ids.h"
#include "cc_buttons.h"
#include "cc_monitor.h"
#include "delay.h"
#include "drive_fsm.h"
#include "fsm.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
int sockfd = 0;
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 9 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 8 }

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

void pre_loop_init() {
  Task *cc_buttons_master_task = get_master_task();
  init_cc_buttons(cc_buttons_master_task);
  init_drive_fsm();
  dashboard_init();
}

void run_fast_cycle() {
  get_button_press();
  update_displays();
}

void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  uint32_t notif = 0;
  notify_get(&notif);
  update_indicators(notif);
  monitor_cruise_control();
  fsm_run_cycle(drive);
  wait_tasks(1);

  update_drive_output(notif);
  run_can_tx_cycle();
  wait_tasks(1);

  // #ifdef x86
  // sim_init(sockfd);
  // #endif
}

void run_slow_cycle() {}
#ifdef x86
int main(int argc, char *argv[]) {
  x86_main_init(atoi(argv[1]));
#else 
int main() {
#endif
  log_init();
  tasks_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_STANDARD,
    .sda = I2C1_SDA,
    .scl = I2C1_SCL,
  };
  i2c_init(I2C_PORT_1, &i2c_settings);
  can_init(&s_can_storage, &can_settings);

  LOG_DEBUG("Welcome to TEST! \n");

  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main? \n");
  return 0;
}
