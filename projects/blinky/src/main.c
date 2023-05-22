#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "gpio.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "wait.h"
#include "delay.h"

// Controller Board LEDs
static const GpioAddress leds[] = {
   { .port = GPIO_PORT_B, .pin = 5 },
   { .port = GPIO_PORT_B, .pin = 4 },
   { .port = GPIO_PORT_B, .pin = 3 },
   { .port = GPIO_PORT_B, .pin = 15 },
};

// Buttons
static const GpioAddress buttons[] = {
   { .port = GPIO_PORT_A, .pin = 7 },
};

// Interrupt Settings
static InterruptSettings interrupt_settings = {
   .type = INTERRUPT_TYPE_INTERRUPT,
   .priority = INTERRUPT_PRIORITY_NORMAL,
};

// Callback Function
void prv_my_gpio_it_callback(GpioAddress *address, void *context) {
   for (size_t i = 0; i <  SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
   }
}

int main() {
   // Enables
   interrupt_init();
   gpio_init();
   gpio_it_register_interrupt(&buttons[0], &interrupt_settings,
                           INTERRUPT_EDGE_RISING, prv_my_gpio_it_callback,
                           NULL);

   // LEDs Config
   GpioSettings led_settings = {
      .direction = GPIO_DIR_OUT,          // Output Pin
      .state = GPIO_STATE_LOW,            // Start in low state
      .alt_function = GPIO_ALTFN_NONE,     // No connections to peripherals
      .resistor = GPIO_RES_NONE,          // No resistors needed
   };

   // Button Config
   GpioSettings button_settings = {
      .direction = GPIO_DIR_IN,          // Output Pin
      .state = GPIO_STATE_LOW,            // Start in low state
      .alt_function = GPIO_ALTFN_NONE,     // No connections to peripherals
      .resistor = GPIO_RES_PULLDOWN,          // No resistors needed
   };

   // Init all the LEDs
   for (size_t i = 0; i <  SIZEOF_ARRAY(leds); i++) {
      gpio_init_pin(&leds[i], &led_settings);
   }

   while (true) {
      wait();
      gpio_it_trigger_interrupt();
      delay_ms(100);
   }

   return 0;
}

