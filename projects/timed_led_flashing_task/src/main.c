#include <stdio.h>
#include <unistd.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

// globals
const GpioSettings settings_out = { GPIO_DIR_OUT, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
const InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

const GpioAddress address1 = { GPIO_PORT_B, 5 };
const GpioAddress address2 = { GPIO_PORT_B, 4 };
const GpioAddress address3 = { GPIO_PORT_B, 3 };
const GpioAddress address4 = { GPIO_PORT_A, 15 };

void prv_my_timer_callback(SoftTimerId timer_id, void *context) {
  gpio_toggle_state(&address1);
  gpio_toggle_state(&address2);
  gpio_toggle_state(&address3);
  gpio_toggle_state(&address4);
}

int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  gpio_init_pin(&address1, &settings_out);
  gpio_init_pin(&address2, &settings_out);
  gpio_init_pin(&address3, &settings_out);
  gpio_init_pin(&address4, &settings_out);

  while (true) {
    soft_timer_start(5000000, (SoftTimerCallback)prv_my_timer_callback, NULL, NULL);
    delay_s(10);  // sleep for 10 seconds so we don't create a new timer
  }

  return 0;
}
