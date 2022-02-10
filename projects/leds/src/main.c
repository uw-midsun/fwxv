#include "gpio.h"
#include "leds_task.h"
#include "log.h"
#include "tasks.h"

int main(void) {
  gpio_init();

  tasks_init_task(&leds_task, 2, NULL);

  LOG_DEBUG("Blinking LEDs...\n");
  tasks_start();

  return 0;
}
