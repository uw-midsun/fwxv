#include "pedal_data.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "pedal_calib.h"
#include "status.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2
#define EE_PEDAL_VALUE_DENOMINATOR ((1 << 12))
Max11600Storage s_max11600_storage = { 0 };
PedalCalibBlob global_calib_blob;

StatusCode read_pedal_data(int16_t *reading, MAX11600Channel channel) {
  status_ok_or_return(max11600_read_converted(&s_max11600_storage));
  PedalCalibBlob *calib_blob = &global_calib_blob;
  int32_t range = calib_blob->brake_calib.upper_value - calib_blob->brake_calib.lower_value;

  *reading = (int16_t)s_max11600_storage.channel_readings[channel];
  int32_t reading_upscaled = (int32_t)*reading * EE_PEDAL_VALUE_DENOMINATOR;
  reading_upscaled *= 100;

  if (range != 0) {
    reading_upscaled /= (range * EE_PEDAL_VALUE_DENOMINATOR);
    *reading = (int16_t)reading_upscaled;
  }
  return STATUS_CODE_OK;
};

StatusCode read_throttle_data(int16_t *reading) {
  return read_pedal_data(reading, THROTTLE_CHANNEL);
}

StatusCode read_brake_data(int16_t *reading) {
  return read_pedal_data(reading, BRAKE_CHANNEL);
}
