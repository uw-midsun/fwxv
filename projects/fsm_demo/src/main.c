#include "delay.h"
#include "fsm.h"
#include "fsm1.h"
#include "fsm2.h"
#include "log.h"
#include "tasks.h"

TASK(master_task, TASK_STACK_512) {
  while (true) {
    fsm_run_cycle(fsm1);
    fsm_run_cycle(fsm2);
    delay_ms(1000);
  }
}

int main(void) {
  log_init();
  init_fsm1();
  init_fsm2();
  tasks_init_task(master_task, TASK_PRIORITY(3), NULL);

  LOG_DEBUG("FSM Demo...\n");
  tasks_start();

  return 0;
}
