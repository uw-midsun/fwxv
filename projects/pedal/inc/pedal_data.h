#pragma once
#include "max11600.h"

#define BRAKE_LIMIT_SWITCH \
  { .port = GPIO_PORT_A, .pin = 7 }

#define ADC_HALL_SENSOR \
  { .port = GPIO_PORT_A, .pin = 1 }

// Read the throttle value
StatusCode read_throttle_data(uint32_t *position);

void pedal_data_init();
