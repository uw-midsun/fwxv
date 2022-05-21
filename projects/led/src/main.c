#include <stdbool.h>
#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "wait.h"
int main(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  // leds intializations
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

  // button initializations
  const GpioAddress button_address = { .port = GPIO_PORT_A, .pin = 7 };
  const GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLDOWN,
    .alt_function = GPIO_ALTFN_NONE,
  };
  // interrupts initializations
  const InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  const InterruptEdge interrupt_edge = { INTERRUPT_EDGE_RISING };

  // timer initializations
  // soft_timer_start(uint32_t duration_ms, SoftTimerCallback soft_timer_callback, SoftTimer *timer)
  // void soft_timer_callback(GpioAddress * button_address, void *context)) {

  // }

  void prv_my_gpio_it_callback(GpioAddress * button_address, void *context) {
    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
      delay_ms(500);
    }
  }
  gpio_it_register_interrupt(
      const GpioAddress *button_address, const InterruptSettings *interrupt_settings,
      InterruptEdge interrupt_edge, GpioItCallback prv_my_gpio_it_callback, void *context);

  // for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
  //   prv_my_gpio_it_callback(leds[i], NULL);
  // }

  while (true) {
    // #ifdef MS_PLATFORM_X86
    //     LOG_DEBUG("blink\n");
    // #endif
    //     for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    //       gpio_toggle_state(&leds[i]);
    //       delay_ms(500);
    //     }
    wait();
  }
  return 0;
}
