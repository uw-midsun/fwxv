#include <stdbool.h>
#include <stdio.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "misc.h"
#include "tasks.h"

static const GpioAddress smoke_gpio[] = {
  {
      .port = GPIO_PORT_A,
      .pin = 0,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 1,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 2,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 3,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 4,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 5,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 6,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 7,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 8,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 9,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 10,
  },
  // Pins A11-14 used for SWD/CAN
  {
      .port = GPIO_PORT_A,
      .pin = 15,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 0,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 1,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 2,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 3,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 4,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 5,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 6,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 7,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 8,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 9,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 10,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 11,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 12,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 13,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 14,
  },
  {
      .port = GPIO_PORT_B,
      .pin = 15,
  },
  {
      .port = GPIO_PORT_C,
      .pin = 13,
  },
};

TASK(smoke_gpio_task, TASK_STACK_512) {
  for (uint8_t i = 0; i < SIZEOF_ARRAY(smoke_gpio); i++) {
    gpio_init_pin(&smoke_gpio[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  }
  while (true) {
    for (uint8_t i = 0; i < SIZEOF_ARRAY(smoke_gpio); i++) {
      gpio_toggle_state(&smoke_gpio[i]);
    }
    delay_s(1);
    for (uint8_t i = 0; i < SIZEOF_ARRAY(smoke_gpio); i++) {
      gpio_toggle_state(&smoke_gpio[i]);
    }
    delay_s(3);
  }
}

int main(void) {
  tasks_init();
  log_init();
  gpio_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(smoke_gpio_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("Tasks Ended\n");
  return 0;
}