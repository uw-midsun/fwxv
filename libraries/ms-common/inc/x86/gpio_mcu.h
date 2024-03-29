#pragma once

#define GPIO_PINS_PER_PORT 16

typedef enum {
  GPIO_PORT_A = 0,
  GPIO_PORT_B,
  GPIO_PORT_C,
  GPIO_PORT_D,
  GPIO_PORT_E,
  GPIO_PORT_F,
  GPIO_PORT_G,
  NUM_GPIO_PORTS,
} GpioPort;

#define GPIO_TOTAL_PINS ((GPIO_PINS_PER_PORT) * (NUM_GPIO_PORTS))
