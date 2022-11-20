#include <stdio.h>
#include <unistd.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

int main() {
  gpio_init();

  const GpioSettings settings = { GPIO_DIR_OUT, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  const GpioAddress address1 = { GPIO_PORT_B, 5 };
  const GpioAddress address2 = { GPIO_PORT_B, 4 };
  const GpioAddress address3 = { GPIO_PORT_B, 3 };
  const GpioAddress address4 = { GPIO_PORT_A, 15 };

  gpio_init_pin(&address1, &settings);
  gpio_init_pin(&address2, &settings);
  gpio_init_pin(&address3, &settings);
  gpio_init_pin(&address4, &settings);

  while (1) {
    gpio_toggle_state(&address1);
    delay_s(1);
    gpio_toggle_state(&address2);
    delay_s(1);
    gpio_toggle_state(&address3);
    delay_s(1);
    gpio_toggle_state(&address4);
    delay_s(1);
  }

  return 0;
}
