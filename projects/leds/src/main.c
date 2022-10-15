#include <stdbool.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

TASK(leds_task, TASK_STACK_512) {
  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  }

  while (true) {
#ifdef MS_PLATFORM_X86
    LOG_DEBUG("blink\n");
#endif
    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
      delay_ms(50);
    }
  }
}

int main(void) {
  gpio_init();
  log_init();

  tasks_init_task(leds_task, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Blinking LEDs...\n");
  tasks_start();

  return 0;
}
