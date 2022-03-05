#include <stdbool.h>

<<<<<<< HEAD
#include "FreeRTOS.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "task.h"

static StackType_t s_task_stack;
static StaticTask_t s_task_tcb;
=======
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"
>>>>>>> main

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

<<<<<<< HEAD
static void blink_task(void *params) {
  TickType_t last_execution_time = xTaskGetTickCount();
  while (true) {
#ifdef MS_PLATFORM_X86
    LOG_DEBUG("blink\n");
#endif  // MS_PLATFORM_X86
    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
      vTaskDelayUntil(&last_execution_time, pdMS_TO_TICKS(50));
=======
TASK(leds_task, TASK_STACK_512) {
  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,        //
    .state = GPIO_STATE_HIGH,         //
    .alt_function = GPIO_ALTFN_NONE,  //
    .resistor = GPIO_RES_NONE,        //
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  while (true) {
#ifdef MS_PLATFORM_X86
    LOG_DEBUG("blink\n");
#endif
    for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
      delay_ms(50);
>>>>>>> main
    }
  }
}

int main(void) {
<<<<<<< HEAD
  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,        //
    .state = GPIO_STATE_HIGH,         //
    .alt_function = GPIO_ALTFN_NONE,  //
    .resistor = GPIO_RES_NONE,        //
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  xTaskCreateStatic(blink_task, "Blink LED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,
                    &s_task_stack, &s_task_tcb);

  LOG_DEBUG("Blinking LEDs...\n");
  vTaskStartScheduler();
=======
  gpio_init();

  tasks_init_task(leds_task, TASK_PRIORITY(2), NULL);

  LOG_DEBUG("Blinking LEDs...\n");
  tasks_start();
>>>>>>> main

  return 0;
}
