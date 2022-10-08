#include <stdio.h>

#include "log.h"
#include "tasks.h"
#include "delay.h"
#include "gpio.h"
#include "misc.h"
#include "soft_timer.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

static uint8_t done_timer = 1;

static const GpioAddress leds[] = {
   { .port = GPIO_PORT_B, .pin = 5 },   //
   { .port = GPIO_PORT_B, .pin = 4 },   //
   { .port = GPIO_PORT_B, .pin = 3 },   //
   { .port = GPIO_PORT_A, .pin = 15 },  //
};

static SoftTimer s_timer;

void blink_leds(SoftTimerId id) {
  for(uint8_t i = 0; i < 4; ++i){
    gpio_toggle_state(&leds[i]);
  }
  LOG_DEBUG("toggled leds\n");
  // done_timer = 1;
  // soft timer start
  soft_timer_start(500, blink_leds, &s_timer);

}

TASK(master_task, TASK_MIN_STACK_SIZE) {

  LOG_DEBUG("in master task 1\n");
  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,        //
    .state = GPIO_STATE_HIGH,         //
    .alt_function = GPIO_ALTFN_NONE,  //
    .resistor = GPIO_RES_NONE,        //
  };
  LOG_DEBUG("in master task 2\n");
  for(uint8_t i = 0; i < SIZEOF_ARRAY(leds); ++i){
    gpio_init_pin(&leds[0], &led_settings);
  }

  LOG_DEBUG("in master task 2\n");
  soft_timer_start(500, blink_leds, &s_timer);
  while(true){
    // delay_ms(100);
    // non_blocking_delay_ms(100);

  }

  // while (true) {
  //   // LOG_DEBUG("in while loop\n");
  //   if(done_timer == 1) {
  //     done_timer = 0;
  //   }
  // }
}

int main() {
  log_init();
  gpio_init();
  tasks_init();

  tasks_init_task(master_task, TASK_PRIORITY(1), NULL);

  LOG_DEBUG("Welcome to led_blink_102!\n");

  tasks_start();
  
  LOG_DEBUG("done!\n");

  return 0;
}
