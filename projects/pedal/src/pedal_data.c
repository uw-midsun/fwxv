#include "pedal_data.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "status.h"

#define THROTTLE_CHANNEL ADS1015_CHANNEL_0
#define BRAKE_CHANNEL ADS1015_CHANNEL_2
#define EE_PEDAL_VALUE_DENOMINATOR ((1 << 12))
Ads1015Storage s_ads1015_storage = { 0 };

// TODO(Scrubpai): Read any GPIOs and the ADC for throttle
StatusCode read_throttle_data(int16_t *position) {
  status_ok_or_return(ads1015_read_raw(&s_ads1015_storage, THROTTLE_CHANNEL, position));
  int32_t position_upscaled = (int32_t)*position * EE_PEDAL_VALUE_DENOMINATOR;
  int32_t range = 0;
  // TODO(Scrubpai): Add calibration values and update position_upscaled (using range)
  position_upscaled *= 100;
  if (range != 0) {
    position_upscaled /= (range * EE_PEDAL_VALUE_DENOMINATOR);
    *position = (int16_t)position_upscaled;
  }
  return STATUS_CODE_OK;
}

// TODO(Scrubpai): Read any GPIOs and the ADC for brake
bool read_brake_data(int16_t *position) {
  ads1015_read_raw(&s_ads1015_storage, BRAKE_CHANNEL, position);
  int32_t position_upscaled = (int32_t)*position * EE_PEDAL_VALUE_DENOMINATOR;
  int32_t range = 0;
  // TODO(Scrubpai): Add calibration values and update position_upscaled (using range)
  position_upscaled *= 100;
  if (range != 0) {
    position_upscaled /= (range * EE_PEDAL_VALUE_DENOMINATOR);
    *position = (int16_t)position_upscaled;
  }
  if (*position > 0) {
    return true;
  }
  return false;
}