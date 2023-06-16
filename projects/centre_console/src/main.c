#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "delay.h"
#include "drive_fsm.h"
#include "fsm.h"
#include "fsm_shared_mem.h"
#include "log.h"
// #include "master_task.h"
#include "power_fsm.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

FSMStorage cc_storage;

void run_fast_cycle() {}

void run_medium_cycle() {
  run_can_rx_cycle();
  fsm_run_cycle(drive);
  fsm_run_cycle(centre_console_power_fsm);
  wait_tasks(1);
  run_can_tx_cycle();
  // delay_ms(1000);
}

void run_slow_cycle() {}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
    run_fast_cycle();
    if (!(counter % 10)) run_medium_cycle();
    if (!(counter % 100)) run_slow_cycle();
    vTaskDelay(pdMS_TO_TICKS(1000));
    ++counter;
  }
}

int main() {
  log_init();
  tasks_init();
  can_init(&s_can_storage, &can_settings);

  LOG_DEBUG("Welcome to TEST! \n");
  fsm_shared_mem_init(&cc_storage);

  init_drive_fsm();
  init_power_fsm();

  // init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main? \n");
  return 0;
}
