#include <stdio.h>
#include <time.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },
  { .port = GPIO_PORT_B, .pin = 4 },
  { .port = GPIO_PORT_B, .pin = 3 },
  { .port = GPIO_PORT_A, .pin = 15 },
};

void gpio_it_callback(SoftTimerId timer_id, void *context) {
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_toggle_state(&leds[i]);
  }
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  // Led Configuration Settings
  GpioSettings ledSettings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  // Initialize Pins
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &ledSettings);
  }

  // soft_timer_start(1000000, gpio_it_callback, NULL, NULL);

  while (true) {
    wait();
    soft_timer_start(1000000, gpio_it_callback, NULL, NULL);
    delay_ms(1000);
  }

  return 0;
}
