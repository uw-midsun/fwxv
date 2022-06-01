
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

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  // Led Configuration Settings
  GpioSettings ledSettings = {
    .direction = GPIO_DIR_OUT,        // LEDs are output
    .state = GPIO_STATE_HIGH,         // LEDS start off
    .resistor = GPIO_RES_NONE,        // default state (resistor)
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  // Initialize Pins
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &ledSettings);
  }

  // Toggle State
  while (true) {
    for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
    }
    delay_ms(1000);
  }

  return 0;
}
