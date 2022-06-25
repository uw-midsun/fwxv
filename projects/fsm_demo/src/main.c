#include "fsm.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"

#include "fsm1.h"
#include "fsm2.h"

TASK(master_task, TASK_STACK_512) {
  // const GpioSettings led_settings = {
  //   .direction = GPIO_DIR_OUT,        //
  //   .state = GPIO_STATE_HIGH,         //
  //   .alt_function = GPIO_ALTFN_NONE,  //
  //   .resistor = GPIO_RES_NONE,        //
  // };

  // for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
  //   gpio_init_pin(&leds[i], &led_settings);
  // }
  LOG_DEBUG(" in master task\n");

  while (true) {
    fsm_run_cycle(fsm1);
    fsm_run_cycle(fsm2);
    delay_ms(1000);
  }
// #ifdef MS_PLATFORM_X86
//     LOG_DEBUG("blink\n");
// #endif
//     for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
//       gpio_toggle_state(&leds[i]);
//       delay_ms(50);
//     }
//   }
}

int main(void) {
  //gpio_init();
  log_init();
  init_fsm1();
  init_fsm2();
  tasks_init_task(master_task, TASK_PRIORITY(3), NULL);

  LOG_DEBUG("Blinking LEDs...\n");
  tasks_start();

  return 0;
}
