#include <stdbool.h>
#include <stddef.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "wait.h"

static GpioAddress address1 = { .port = GPIO_PORT_B, .pin = 5 };
static GpioAddress address2 = { .port = GPIO_PORT_B, .pin = 4 };
static GpioAddress address3 = { .port = GPIO_PORT_B, .pin = 3 };
static GpioAddress address4 = { .port = GPIO_PORT_A, .pin = 15 };

void prv_my_gpio_it_callback(SoftTimerId id, void *context) {
  gpio_toggle_state(&address1);
  gpio_toggle_state(&address2);
  gpio_toggle_state(&address3);
  gpio_toggle_state(&address4);
  soft_timer_start(1000000, prv_my_gpio_it_callback, NULL, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  InterruptSettings int_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                     .priority = INTERRUPT_PRIORITY_NORMAL };

  GpioSettings pin_settings = { .direction = GPIO_DIR_OUT,
                                .state = GPIO_STATE_LOW,
                                .alt_function = GPIO_ALTFN_NONE,
                                .resistor = GPIO_RES_NONE };

  GpioSettings button_settings = { .direction = GPIO_DIR_IN,
                                   .state = GPIO_STATE_LOW,
                                   .alt_function = GPIO_ALTFN_NONE,
                                   .resistor = GPIO_RES_PULLDOWN };

  gpio_init_pin(&address1, &pin_settings);
  gpio_init_pin(&address2, &pin_settings);
  gpio_init_pin(&address3, &pin_settings);
  gpio_init_pin(&address4, &pin_settings);
  soft_timer_start(1000000, prv_my_gpio_it_callback, NULL, NULL);

  while (true) {
    wait();
  }
}
