#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// Callback function
void gpio_callback(const GpioAddress *address, void *context) {
  LOG_DEBUG("The callback was triggered.\n");
  GpioAddress *led_address = (GpioAddress *)context;
  gpio_toggle_state(led_address);  // Toggles LED connected to B5
}

int main(void) {
  interrupt_init();
  // soft_timer_init();
  gpio_init();

  // Struct for LED pins
  const GpioAddress leds[] = {
    { .port = GPIO_PORT_B, .pin = 5 },
    { .port = GPIO_PORT_B, .pin = 4 },
    { .port = GPIO_PORT_B, .pin = 3 },
    { .port = GPIO_PORT_A, .pin = 15 },
  };

  // Struct for Button pins
  const GpioAddress buttons[] = { { .port = GPIO_PORT_A, .pin = 7 } };

  // Struct for LED settings
  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };

  // Struct for Button settings
  const GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_PULLDOWN,
  };

  // Struct for interrupt settings
  const InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  // Initialize LEDS and BUTTONS
  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }
  gpio_init_pin(&buttons[0], &button_settings);

  // Register interrupt
  gpio_it_register_interrupt(&buttons[0], &interrupt_settings, INTERRUPT_EDGE_RISING, gpio_callback,
                             &leds[0]);

  while (true) {
    // wait(); // This is for hardware testing

    // This is for remote testing purposes:
    gpio_it_trigger_interrupt(&buttons[0]);
  }

  return 0;
}
