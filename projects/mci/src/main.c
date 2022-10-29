#include "delay.h"
#include "fsm.h"
#include "log.h"
#include "mci_fsm.h"
#include "misc.h"
#include "soft_timer.h"
#include "tasks.h"

TASK(master_task, TASK_STACK_512) {
  while (true) {
    fsm_run_cycle(mci_fsm);
    delay_ms(100);
  }
}

int main(void) {
  tasks_init();
  log_init();
  init_mci_fsm();
  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("MCI FSM...\n");
  tasks_start();

  return 0;
}
