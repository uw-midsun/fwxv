#include <stdio.h>

#include "delay.h"
#include "log.h"
#include "misc.h"
#include "notify.h"
#include "tasks.h"

void pre_loop_init() {}

  static int prv_my_func(int *input) {  // Function declarations/definitions
    *input += 1;
    return *input;  // Increments the value passed and returns it
  }

TASK(helloworld_task, TASK_STACK_512) {

  int num = 0;
  while (1) {
    LOG_DEBUG("Hello World %d\n", prv_my_func(&num));
    delay_ms(100);
  }
}

int main(void) {
  tasks_init();
  log_init();

  tasks_init_task(helloworld_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  return 0;
}
