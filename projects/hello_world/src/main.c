#include <stdbool.h>
#include <stdio.h>

#include "gpio.h"
#include "interrupt.h"
//#include "wait.h"
#include "delay.h"
#include "gpio_it.h"
#include "soft_timer.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

static GpioSettings led_settings = {
  .direction = GPIO_DIR_OUT,        //
  .state = GPIO_STATE_LOW,          //
  .alt_function = GPIO_ALTFN_NONE,  //
  .resistor = GPIO_RES_NONE,        //
};

static void prv_my_gpio_it_callback(SoftTimerId timer_id, void *context) {
  GpioAddress *led = context;
  gpio_toggle_state(led);
  soft_timer_start(1000000, prv_my_gpio_it_callback, led);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
    soft_timer_start(1000000, prv_my_gpio_it_callback, &leds[i]);
  }

  while (true) {
    wait();
  }

  return 0;
}