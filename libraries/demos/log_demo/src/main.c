#include <stdbool.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"


TASK(break_task, TASK_STACK_512) {
  while(true){
    LOG_DEBUG("THIS IS THE FIRST TASK AND IT SHOULD NOT BE INTERRUPTED AND IT IS IN ALL CAPS, AND THIS SHOULD RUN\r\n");
    delay_ms(10);
  }
}

TASK(break2_task, TASK_STACK_512) {
  while(true){
    LOG_DEBUG("this is the second task and it should also not be interrupted and it is not in all caps, and this should run\r\n");
    delay_ms(10);
  }
}

int main(void) {
  tasks_init_task(break_task, TASK_PRIORITY(2), NULL);
  tasks_init_task(break2_task, TASK_PRIORITY(2), NULL);

  log_init();

  tasks_start();

  return 0;
}
