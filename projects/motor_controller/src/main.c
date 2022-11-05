#include "can.h"
#include "can_board_ids.h"
#include "delay.h"
#include "fsm.h"
#include "log.h"
#include "mci_fsm.h"
#include "misc.h"
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

TASK(master_task, TASK_STACK_512) {
  while (true) {
    fsm_run_cycle(mci_fsm);
    delay_ms(100);
  }
}

int main(void) {
  tasks_init();
  log_init();

  can_init(&s_can_storage, &can_settings);
  can_add_filter_in(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1);

  init_mci_fsm();
  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Motor Controller Task...\n");
  tasks_start();

  return 0;
}
