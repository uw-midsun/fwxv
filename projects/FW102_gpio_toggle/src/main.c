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

void wait_using_delay(void) {
  delay_ms(1000);  // Delay causes segmentation fault?
}

int main() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  // soft_timer_init();

  const GpioAddress leds[] = {
    { .port = GPIO_PORT_B, .pin = 5 },
    { .port = GPIO_PORT_B, .pin = 4 },
    { .port = GPIO_PORT_B, .pin = 3 },
    { .port = GPIO_PORT_A, .pin = 15 },
  };

  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  while (true) {
    LOG_DEBUG("BLINK!\n");
    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);  // Toggles all LEDS on
    }
    wait_using_delay();  // Waits using the delay_ms function
  }
  return 0;
}
