#include <stdbool.h>
#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static int my_int = 0;
static GpioSettings gpio_settings = {
  .direction = GPIO_DIR_OUT,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_NONE,
};

const GpioSettings button_settings = {
  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_PULLDOWN,
  .alt_function = GPIO_ALTFN_NONE,
};

const GpioAddress LEDS[] = {
  { .port = GPIO_PORT_B, .pin = 5 },
  { .port = GPIO_PORT_B, .pin = 4 },
  { .port = GPIO_PORT_B, .pin = 3 },
  { .port = GPIO_PORT_A, .pin = 15 },
};
const GpioAddress button = { .port = GPIO_PORT_A, .pin = 7 };

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

void prv_init_gpio_pins(void) {
  for (uint8_t i = 0; i < SIZEOF_ARRAY(LEDS); i++) {
    gpio_init_pin(&LEDS[i], &gpio_settings);
  }
  // gpio_init_pin(&button, &button_settings);
}

// void prv_register_interrupts(void) {
//    gpio_it_register_interrupt(&button, &s_interrupt_settings, INTERRUPT_EDGE_RISING,
//    prv_my_gpio_callback,
//                               &LEDS[0]);
//    // gpio_it_register_interrupt(&button, &s_interrupt_settings, INTERRUPT_EDGE_RISING,
//    prv_my_gpio_callback,
//    //                            &LEDS[1]);
// }

void prv_my_gpio_callback(const GpioAddress *address, void *context) {
  GpioAddress *LED = (GpioAddress *)context;
  gpio_toggle_state(LED);
  // LOG_DEBUG("Task %i\n", my_int);
  my_int++;
}
void callback(SoftTimerId timer_id, void *context) {
  GpioAddress *btn = (GpioAddress *)context;
  gpio_it_trigger_interrupt(btn);
  soft_timer_start(1000000, callback, &button, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  prv_init_gpio_pins();
  gpio_it_register_interrupt(&button, &s_interrupt_settings, INTERRUPT_EDGE_RISING,
                             prv_my_gpio_callback, &LEDS[0]);

  soft_timer_start(1000000, callback, &button, NULL);

  while (true) {
    wait();
  }

  return 0;
}

// TASK(task_main, TASK_STACK_512) {
//    while (true) {
//       for (uint8_t i = 0; i < SIZEOF_ARRAY(LEDS); i++) {
//          gpio_toggle_state(&LEDS[i]);
//          delay_ms(1000);
//       }
//    }
// }
