#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

int main(void) {
  printf("starting \n");
  //gpio_init();
  //log_init();

  while (true) {
    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      printf("working]\n");
    }
  }

   return 0;
}
