#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

const GpioAddress led1 = { .port = GPIO_PORT_A, .pin = 5 };
const GpioAddress led2 = { .port = GPIO_PORT_B, .pin = 4 };
const GpioAddress led3 = { .port = GPIO_PORT_B, .pin = 3 };
const GpioAddress led4 = { .port = GPIO_PORT_B, .pin = 15 };

// part 2
static void callback(const GpioAddress *address, void *context) {
  gpio_toggle_state(&led1);
  gpio_toggle_state(&led2);
  gpio_toggle_state(&led3);
  gpio_toggle_state(&led4);
}

int main() {
  // part 1, 2
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  // GpioSetting for part 2
  GpioSettings settings;
  settings.direction = GPIO_DIR_IN;
  settings.state = GPIO_STATE_LOW;
  settings.resistor = GPIO_RES_PULLDOWN;
  settings.alt_function = GPIO_ALTFN_NONE;

  // part 2
  static InterruptSettings interrupt_settings;
  interrupt_settings.type = INTERRUPT_TYPE_INTERRUPT;
  interrupt_settings.priority = INTERRUPT_PRIORITY_NORMAL;

  // part 1, 2
  gpio_init_pin(&led1, &settings);
  gpio_init_pin(&led2, &settings);
  gpio_init_pin(&led3, &settings);
  gpio_init_pin(&led4, &settings);

  // part 2
  gpio_it_register_interrupt(&led1, &interrupt_settings, INTERRUPT_EDGE_FALLING, callback, NULL);

  while (true) {
    gpio_it_trigger_interrupt(&led1);
    delay_ms(1000);
  }

  return 0;
}