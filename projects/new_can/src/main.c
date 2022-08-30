#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "log.h"
#include "new_can_setters.h"
#include "tasks.h"

void wait_tasks(int t)
{
  for (size_t i = 0; i < t; ++i)
    wait(&s_end_task_sem);
}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
#ifdefine TEST
    xSemaphoreTake(test_cycle_start_sem);
#endif
    run_fast_cycle();
    if (!(counter % 10))
      run_medium_counter();
    if (!(counter % 100))
      run_slow_counter();

#ifdefine TEST
    xSemaphoreGive(test_cycle_end_sem);
#endif
    vTaskDelay(pdMS_TO_TICKS(1000));
    ++counter;
  }
}

void run_fast_cycle()
{
  run_critical_tasks();
  wait_tasks(1);
}

void run_medium_cycle()
{
  run_can_rx_cycle();
  wait_tasks(1);
  run_gpio_cycle();
  run_test_cycle();
  run_help_cycle();
  wait_tasks(3);
  run_last_cycle();
  wait_tasks(1);
  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle()
{

}

int main() {
  log_init();
  
  LOG_DEBUG("Welcome to CAN!\n");
  can_init(&s_can_storage, &can_settings);
  // can_add_filter(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1);

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
