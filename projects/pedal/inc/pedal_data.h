#pragma once
#include "max11600.h"

// Read the trottle value
// Requires the MAX11600 to be initialized
StatusCode read_throttle_data(uint32_t *position);

// Read the brake value
// Requires the MAX11600 to be initialized
StatusCode read_brake_data(uint32_t *position);

StatusCode read_pedal_data(uint32_t *reading, MAX11600Channel channel);

void pedal_data_init();
