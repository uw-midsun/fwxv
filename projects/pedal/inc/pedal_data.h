#pragma once
#include "ads1015.h"

extern Ads1015Storage s_ads1015_storage;

// Read any GPIOs and the ADC for throttle
StatusCode read_throttle_data(int16_t *position);

// Read any GPIOs and the ADC for brake
bool read_brake_data(int16_t *position);