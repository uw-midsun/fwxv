#include "delay.h"
#include "fsm.h"
#include "fsm1.h"
#include "fsm2.h"
#include "log.h"
#include "tasks.h"
#include "master_task.h"

// TASK(master_task, TASK_STACK_512) {
//   while (true) {
//     fsm_run_cycle(fsm1);
//     wait_tasks(1);
//     fsm_run_cycle(fsm2);
//     wait_tasks(1);
//     delay_ms(1000);
//   }
// }

void pre_loop_init() {}
void run_slow_cycle() {}
void run_fast_cycle() {}

void run_medium_cycle() {
  fsm_run_cycle(fsm1);
  wait_tasks(1);
  fsm_run_cycle(fsm2);
  wait_tasks(1);
  delay_ms(1000);
}

int main(void) {
  log_init();
  tasks_init();
  init_fsm1();
  init_fsm2();
  init_master_task();
  // tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("FSM Demo...\n");
  tasks_start();

  return 0;
}
