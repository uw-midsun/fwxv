#include <stdbool.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"

static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 4 },  //
  { .port = GPIO_PORT_B, .pin = 3 },  //
};

static const GpioAddress buttons[] = {
  { .port = GPIO_PORT_A, .pin = 7 },  // valid buttons?
  { .port = GPIO_PORT_A, .pin = 6 },  //
  { .port = GPIO_PORT_A, .pin = 0 },  // dummy buttons
  { .port = GPIO_PORT_A, .pin = 1 },  // use for interrupt trigger from code
};

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

TASK(leds_task, TASK_STACK_512) {
  while (true) {
    uint32_t value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (int i = 0; i < 3; ++i) {
      if ((value & (1u << i)) != 0) {
        gpio_toggle_state(&leds[i]);
      }
    }
  }
}

TASK(periodic_trigger, TASK_STACK_512) {
  while (true) {
    LOG_DEBUG("trigger interrupt\n\r");
    gpio_it_trigger_interrupt(&buttons[2]);
    delay_s(1);
  }
}

int main(void) {
  tasks_init_task(leds_task, TASK_PRIORITY(3), NULL);
  tasks_init_task(periodic_trigger, TASK_PRIORITY(4), NULL);

  gpio_init();
  interrupt_init();
  gpio_it_init();

  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,        //
    .state = GPIO_STATE_HIGH,         //
    .alt_function = GPIO_ALTFN_NONE,  //
    .resistor = GPIO_RES_NONE,        //
  };
  for (uint8_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  gpio_it_register_interrupt(&buttons[0], &s_interrupt_settings, INTERRUPT_EDGE_RISING,
                             leds_task->handle, 0);
  gpio_it_register_interrupt(&buttons[1], &s_interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
                             leds_task->handle, 1);
  gpio_it_register_interrupt(&buttons[2], &s_interrupt_settings, INTERRUPT_EDGE_RISING,
                             leds_task->handle, 2);

  LOG_DEBUG("Blinking LEDs...\n");
  tasks_start();

  return 0;
}
