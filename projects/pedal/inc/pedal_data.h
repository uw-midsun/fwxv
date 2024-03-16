#pragma once
#include "max11600.h"

#define ADC_POT1_BRAKE \
  { .port = GPIO_PORT_A, .pin = 0 }

#define ADC_HALL_SENSOR \
  { .port = GPIO_PORT_A, .pin = 1 }

// Read the throttle value
StatusCode read_throttle_data(uint32_t *position);

// Read the brake value
StatusCode read_brake_data(uint32_t *position);

StatusCode read_pedal_data(uint32_t *reading, MAX11600Channel channel);

void pedal_data_init();
