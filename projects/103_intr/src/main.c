// READ BEFORE WORKING: CURRENT STATUS & REMINDERS

// currently doesn't compile
// master task doesn't seem to be working, so can't use run_slow_cycle
// says: undefined reference to 'init_master_task'

// also, I believe the reason why notification doesn't work is because not actually supposed to make
// variable? prob should ask

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "master_task.h"
#include "misc.h"
#include "tasks.h"

#define BTN_INT_EVENT 5

GpioAddress addr1 = {
  .port = GPIO_PORT_B,
  .pin = 5,
};
GpioAddress addr2 = {
  .port = GPIO_PORT_B,
  .pin = 4,
};
GpioAddress addr3 = {
  .port = GPIO_PORT_B,
  .pin = 3,
};
GpioAddress addr4 = {
  .port = GPIO_PORT_A,
  .pin = 15,
};

InterruptSettings it_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .edge = INTERRUPT_EDGE_RISING,
};
GpioAddress addr_button = {
  .port = GPIO_PORT_A,
  .pin = 7,
};

void pre_loop_init() {}

void run_fast_cycle() {}

void run_medium_cycle() {}

void run_slow_cycle() {
  gpio_it_trigger_interrupt(&addr_button);
  LOG_DEBUG("Slow Cycle Test\n");
}

TASK(task1, TASK_STACK_512) {
  LOG_DEBUG("Task 1 starting\n\n");
  uint32_t notification;

  gpio_init_pin(&addr1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&addr4, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  while (true) {
    notify_get(&notification);
    if (notify_check_event(&notification, BTN_INT_EVENT)) {
      gpio_toggle_state(&addr1);
      LOG_DEBUG("LED 1 Toggled\n");
      gpio_toggle_state(&addr2);
      LOG_DEBUG("LED 2 Toggled\n");
      gpio_toggle_state(&addr3);
      LOG_DEBUG("LED 3 Toggled\n");
      gpio_toggle_state(&addr4);
      LOG_DEBUG("LED 4 Toggled\n");
    }
  }
}

int main(void) {
  gpio_init();
  interrupt_init();
  log_init();
  tasks_init();

  // Create tasks here
  tasks_init_task(task1, TASK_PRIORITY(2), NULL);

  gpio_init_pin(&addr_button, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_it_register_interrupt(&addr_button, &it_settings, BTN_INT_EVENT, task1);

  LOG_DEBUG("Program start...\n");
  // Start the scheduler
  init_master_task();
  tasks_start();

  return 0;
}
