#pragma once
#include "max11600.h"

extern Max11600Storage s_max11600_storage;

// Read the trottle value in mV
// Requires the MAX11600 to be initialized
StatusCode read_throttle_data(int16_t *position);

// Read the trottle value in mV from the MAX11600
// Requires the MAX11600 to be initialized
StatusCode read_brake_data(int16_t *position);
