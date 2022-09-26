#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "log.h"
#include "new_can_setters.h"
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

void run_fast_cycle() {}

void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
#ifdef TEST
    xSemaphoreTake(test_cycle_start_sem);
#endif
    run_fast_cycle();
    if (!(counter % 10)) run_medium_cycle();
    if (!(counter % 100)) run_slow_cycle();

#ifdef TEST
    xSemaphoreGive(test_cycle_end_sem);
#endif
    vTaskDelay(pdMS_TO_TICKS(100));
    ++counter;
  }
}

int main() {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to CAN!\n");
  can_init(&s_can_storage, &can_settings);
  can_add_filter_in(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1);

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
