#include <stdio.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "tasks.h"
#define NUM_BTNS 16

static GpioAddress btns[NUM_BTNS];

static InterruptSettings settings = {
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .type = INTERRUPT_TYPE_INTERRUPT,
  .edge = INTERRUPT_EDGE_RISING,
};

uint8_t get_pin_from_notif(uint32_t notif) {
  for (uint8_t i = 0; i < 32; ++i) {
    if ((1u << i) & notif) {
      return i;
    }
  }
  return 0;
}

char get_port_from_notif(uint32_t notif) {
  return (notif / GPIO_PINS_PER_PORT) + 65;
}

// Use this task to trigger an interrupt and test on x86
// TASK(tester_task, TASK_MIN_STACK_SIZE) {
//   int counter = 0;
//   while (true) {
//     if (!(counter % 10)) {
//       gpio_it_trigger_interrupt(&btns[0]);
//     }
//     vTaskDelay(pdMS_TO_TICKS(100));
//     ++counter;
//   }
// }

TASK(gpio_project, TASK_MIN_STACK_SIZE) {
  uint32_t notif;
  while (true) {
    if (notify_wait(&notif, 100) == STATUS_CODE_OK) {
      uint8_t interrupt_pin = get_pin_from_notif(notif);
      char interrupt_port = get_port_from_notif(notif);
      if (interrupt_pin >= 32) {
        LOG_DEBUG("Invalid Pin! Notif val: %d, port: %c\n", notif, interrupt_port);
      } else {
        LOG_DEBUG("Notification: %c%d\n", interrupt_port, interrupt_pin);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  interrupt_init();
  gpio_it_init();

  LOG_DEBUG("Welcome to GPIO SMOKE IT!\n");

  for (uint8_t i = 0; i < NUM_BTNS; i++) {
    btns[i].port = i / GPIO_PINS_PER_PORT;  // initializes A0 - A15
    btns[i].pin = i % GPIO_PINS_PER_PORT;
  }

  tasks_init_task(gpio_project, TASK_PRIORITY(2), NULL);
  // tasks_init_task(tester_task, TASK_PRIORITY(2), NULL); uncomment to test on x86

  for (uint8_t i = 0; i < NUM_BTNS; i++) {
    gpio_init_pin(&btns[i], GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
    gpio_it_register_interrupt(&btns[i], &settings, i, gpio_project);
  }

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
