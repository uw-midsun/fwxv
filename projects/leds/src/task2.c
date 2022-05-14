// Example program for STM32F072 Controller board or Discovery Board.
// Blinks the LEDs sequentially.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"       // For real-time delays
#include "gpio.h"        // General Purpose I/O control.
#include "interrupt.h"   // For enabling interrupts.
#include "misc.h"        // Various helper functions/macros.
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "gpio_it.h"
#include "log.h"
#include "wait.h"

// Depending on which board you are working with you will need to (un)comment
// the relevant block of GPIO pins. Generally these would be in a configuration
// file somewhere and we normally write configurations only for the controller
// board.

// Controller board LEDs
static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

static const GpioAddress btn = {
   .port = GPIO_PORT_A, .pin = 7 
}; 

// Discovery board LEDs
// static const GpioAddress leds[] = {
//  { .port = GPIO_PORT_C, .pin = 8 },  //
//  { .port = GPIO_PORT_C, .pin = 9 },  //
//  { .port = GPIO_PORT_C, .pin = 6 },  //
//  { .port = GPIO_PORT_C, .pin = 7 },  //
//};


static void prv_gpio_it_cb(const GpioAddress *add, void *context) {
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_toggle_state(&leds[i]);
  }
}


int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,        // The pin needs to output.
    .state = GPIO_STATE_HIGH,         // Start in the "on" state.
    .alt_function = GPIO_ALTFN_NONE,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  // Init all of the LED pins
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  GpioSettings btn_settings = {
    .direction = GPIO_DIR_IN,        
    .alt_function = GPIO_ALTFN_NONE,  
    .resistor = GPIO_RES_PULLDOWN,       
  };
  gpio_init_pin(&btn, &btn_settings);

  InterruptSettings int_set = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&btn, &int_set, INTERRUPT_EDGE_RISING, prv_gpio_it_cb, NULL);
  // Keep toggling the state of the pins from on to off with a 50 ms delay between.
  while (true) {
    wait();
  }

  return 0;
}
