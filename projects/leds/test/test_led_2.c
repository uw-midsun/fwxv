#include <stdbool.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

TASK(leds_task, TASK_STACK_512) {}

bool initialized = false;

void setup_test(void) {
  if (initialized) return;
  initialized = true;

  tasks_init();
  gpio_init();
  log_init();
}

TEST_IN_TASK
void test_leds(void) {
  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  }

  for (int i = 0; i < 9; ++i) {
    LOG_DEBUG("blink\n");
    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
      delay_ms(200);
    }
  }
}
