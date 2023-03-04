#pragma once
#include "max11600.h"

extern Max11600Storage s_max11600_storage;

// Read any GPIOs and the ADC for throttle
StatusCode read_throttle_data(int16_t *position);

// Read any GPIOs and the ADC for brake
StatusCode read_brake_data(int16_t *position);
