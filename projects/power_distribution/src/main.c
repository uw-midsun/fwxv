#include <stdio.h>

#include "can.h"
#include "log.h"
#include "power_seq_fsm.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x05,
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

void run_fast_cycle() {}

void run_medium_cycle() {}

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
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  can_init(&s_can_storage, &can_settings);
  init_power_seq();
  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
