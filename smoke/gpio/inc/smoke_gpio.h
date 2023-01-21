#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"

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
  {
      .port = GPIO_PORT_A,
      .pin = 11,
  },
  {
      .port = GPIO_PORT_A,
      .pin = 12,
  },
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
  {
      .port = GPIO_PORT_C,
      .pin = 14,
  },
  {
      .port = GPIO_PORT_C,
      .pin = 15,
  },
};
