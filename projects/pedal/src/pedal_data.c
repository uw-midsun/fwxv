#include "pedal_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "pedal_calib.h"
#include "pedal_shared_resources_provider.h"
#include "status.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2
#define EE_PEDAL_VALUE_DENOMINATOR ((1 << 12))

static PedalCalibBlob *s_calib_blob;
static Max11600Storage *s_max11600_storage;

static uint32_t prv_get_uint32(float f) {
  union {
    float f;
    uint32_t u;
  } fu = { .f = f };
  return fu.u;
}

void pedal_data_init() {
  s_max11600_storage = get_shared_max11600_storage();
  s_calib_blob = get_shared_pedal_calib_blob();
}

StatusCode read_pedal_data(uint32_t *reading, MAX11600Channel channel) {
  status_ok_or_return(max11600_read_raw(s_max11600_storage));

  int32_t range = s_calib_blob->brake_calib.upper_value - s_calib_blob->brake_calib.lower_value;

  int32_t reading_upscaled =
      (int32_t)s_max11600_storage->channel_readings[channel] * EE_PEDAL_VALUE_DENOMINATOR;
  reading_upscaled -= s_calib_blob->brake_calib.lower_value * EE_PEDAL_VALUE_DENOMINATOR;
  reading_upscaled *= 100;

  if (range != 0) {
    reading_upscaled /= (range * EE_PEDAL_VALUE_DENOMINATOR);
    // Todo(Bafran): Make sure the data type is good with MCI
    *reading = prv_get_uint32(reading_upscaled / 100.0);
  }

  return STATUS_CODE_OK;
}

StatusCode read_throttle_data(uint32_t *reading) {
  return read_pedal_data(reading, THROTTLE_CHANNEL);
}

StatusCode read_brake_data(uint32_t *reading) {
  return read_pedal_data(reading, BRAKE_CHANNEL);
}
