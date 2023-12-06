#pragma once

// Fault pin addresses

// GPIO
#define AUX_FAULT_GPIO_1 \
  { .port = GPIO_PORT_B, .pin = 3 }
#define AUX_FAULT_GPIO_2 \
  { .port = GPIO_PORT_B, .pin = 4 }
#define DCDC_FAULT_GPIO_1 \
  { .port = GPIO_PORT_A, .pin = 15 }
#define DCDC_FAULT_GPIO_2 \
  { .port = GPIO_PORT_B, .pin = 5 }
#define DCDC_FAULT_GPIO_3 \
  { .port = GPIO_PORT_A, .pin = 6 }

// ADC (placeholders for now)
#define AUX_OVERCURRENT \
  { .port = GPIO_PORT_A, .pin = 1 }
#define DCDC_OVERCURRENT \
  { .port = GPIO_PORT_A, .pin = 2 }
#define AUX_OVERTEMP \
  { .port = GPIO_PORT_A, .pin = 3 }
#define DCDC_OVERTEMP \
  { .port = GPIO_PORT_A, .pin = 4 }
