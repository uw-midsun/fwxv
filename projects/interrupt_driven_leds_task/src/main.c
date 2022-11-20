#include <stdio.h>
#include <unistd.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

// globals
const GpioSettings settings_in = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_PULLDOWN,
                                   GPIO_ALTFN_NONE };
const GpioSettings settings_out = { GPIO_DIR_OUT, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
const InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

const GpioAddress address_in = { GPIO_PORT_A, 7 };
const GpioAddress address1 = { GPIO_PORT_B, 5 };
const GpioAddress address2 = { GPIO_PORT_B, 4 };
const GpioAddress address3 = { GPIO_PORT_B, 3 };
const GpioAddress address4 = { GPIO_PORT_A, 15 };

void prv_my_gpio_it_callback(GpioAddress *address, void *context) {
  gpio_toggle_state(&address1);
  gpio_toggle_state(&address2);
  gpio_toggle_state(&address3);
  gpio_toggle_state(&address4);
}

int main() {
  interrupt_init();
  gpio_it_init();
  gpio_init();

  gpio_init_pin(&address_in, &settings_in);
  gpio_init_pin(&address1, &settings_out);
  gpio_init_pin(&address2, &settings_out);
  gpio_init_pin(&address3, &settings_out);
  gpio_init_pin(&address4, &settings_out);

  gpio_it_register_interrupt(&address_in, &it_settings, INTERRUPT_EDGE_RISING,
                             (GpioItCallback)prv_my_gpio_it_callback, NULL);

  while (true) {
    gpio_it_trigger_interrupt(&address_in);
    delay_s(5);
  }

  return 0;
}
