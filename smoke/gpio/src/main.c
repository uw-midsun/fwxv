#include <stdbool.h>
#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "smoke_gpio.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

TASK(smoke_gpio_task, TASK_MIN_STACK_SIZE) {
  for (uint8_t i = 0; i < SIZEOF_ARRAY(smoke_gpio); i++) {
    gpio_init_pin(&smoke_gpio[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  }
  while (true) {
    for (uint8_t i = 0; i < SIZEOF_ARRAY(smoke_gpio); i++) {
      gpio_toggle_state(&smoke_gpio[i]);
      delay_s(1);
    }
  }
}

int main(void) {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(smoke_gpio_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("Pins Toggled\n");
  return 0;
}
