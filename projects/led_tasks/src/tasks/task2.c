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
static const GpioAddress button = { GPIO_PORT_A, 7 };

void gpio_it_callback(const GpioAddress *address, void *context) {
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

  // Button Configuration Settings
  GpioSettings buttonSettings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLDOWN,
    .alt_function = GPIO_ALTFN_NONE,
  };

  // Initialize Pins
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &ledSettings);
  }
  gpio_init_pin(&button, &buttonSettings);

  // Interrupt Settings
  InterruptSettings intSettings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&button, &intSettings, INTERRUPT_EDGE_RISING, gpio_it_callback, NULL);

  while (true) {
    wait();
    gpio_it_trigger_interrupt(&button);
    delay_ms(1000);
  }

  return 0;
}
