#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

// Callback function
void soft_timer_callback(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("The callback was triggered.\n");
  GpioAddress *led_address = (GpioAddress *)context;  // Casts void to our struct so we can use it
  gpio_toggle_state(led_address);                     // Toggles LED connected to B5
  // soft_timer_start(1000, soft_timer_callback, led_address, NULL); // Starts the timer again
}

int main() {
  gpio_init();
  gpio_it_init();
  interrupt_init();
  // soft_timer_init();

  // Struct for LED pins
  const GpioAddress leds[] = {
    { .port = GPIO_PORT_B, .pin = 5 },
    { .port = GPIO_PORT_B, .pin = 4 },
    { .port = GPIO_PORT_B, .pin = 3 },
    { .port = GPIO_PORT_A, .pin = 15 },
  };

  // Struct for LED settings
  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
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

  // Start the timer!
  // soft_timer_start(1000, soft_timer_callback(), &leds[0], NULL); // Fix timer_id.
  soft_timer_start(1000, soft_timer_callback, &leds[0]);  // Fix timer_id.
  // StatusCode soft_timer_start(uint32_t duration_ms, SoftTimerCallback callback, SoftTimer
  // *timer);

  while (true) {
    // wait(); // Waits until an interrupt is triggered
  }

  return 0;
}
