#include <stdio.h>
#include <stdbool.h>
#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "wait.h"
#include "log.h"

void prv_my_gpio_it_callback(const struct GpioAddress *address, void *context);

void register_interrupts(void);

static const GpioAddress leds[] = { { .port = GPIO_PORT_B, .pin = 5 },
                                    { .port = GPIO_PORT_B, .pin = 4 },
                                    { .port = GPIO_PORT_B, .pin = 3 },
                                    { .port = GPIO_PORT_A, .pin = 15 } };

static const GpioAddress button[] = { { .port = GPIO_PORT_A, .pin = 7 } };

static InterruptSettings interrupt_settings={
    .type=INTERRUPT_TYPE_INTERRUPT,
    .priority= INTERRUPT_PRIORITY_NORMAL
};

static GpioSettings led_settings = { .direction = GPIO_DIR_OUT,
                              .state = GPIO_STATE_LOW,
                              .alt_function = GPIO_ALTFN_NONE,
                              .resistor = GPIO_RES_NONE };

static GpioSettings button_settings = { .direction = GPIO_DIR_IN,
                             .state = GPIO_STATE_LOW,
                             .alt_function = GPIO_ALTFN_NONE,
                             .resistor = GPIO_RES_PULLDOWN };

int main() {
  interrupt_init();
  gpio_init();

  gpio_init_pin(&button[0], &button_settings);

  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  register_interrupts();

  while (true) {
    wait();
  }

  return 0;
}

void prv_my_gpio_it_callback(const struct GpioAddress *address, void *context) {
    GpioAddress *led_address=(GpioAddress*) context;
    gpio_toggle_state(led_address);
}

void register_interrupts(void) {
  gpio_it_register_interrupt(&button[0], &interrupt_settings, INTERRUPT_EDGE_RISING, prv_my_gpio_it_callback, &leds[0]);
  gpio_it_register_interrupt(&button[0], &interrupt_settings, INTERRUPT_EDGE_RISING, prv_my_gpio_it_callback, &leds[1]);
  gpio_it_register_interrupt(&button[0], &interrupt_settings, INTERRUPT_EDGE_RISING, prv_my_gpio_it_callback, &leds[2]);
  gpio_it_register_interrupt(&button[0], &interrupt_settings, INTERRUPT_EDGE_RISING, prv_my_gpio_it_callback, &leds[3]);
}
