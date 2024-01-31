#include "pedal_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "log.h"
#include "pedal_calib.h"
#include "pedal_shared_resources_provider.h"
#include "status.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2
#define EE_PEDAL_VALUE_DENOMINATOR ((1 << 12))

static const GpioAddress brake = ADC_POT1_BRAKE;
static const GpioAddress throttle = ADC_HALL_SENSOR;

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
  // Don't call max11600 functions
  s_calib_blob = get_shared_pedal_calib_blob();
}

StatusCode read_pedal_data(uint32_t *reading, MAX11600Channel channel) {
  // Don't call max11600 functions

  uint16_t adc_reading;
  if (channel == BRAKE_CHANNEL) {
    adc_read_raw(brake, &adc_reading);
  } else if (channel == THROTTLE_CHANNEL) {
    adc_read_raw(throttle, &adc_reading);
  }

  *reading = adc_reading;

  int32_t range_brake =
      s_calib_blob->brake_calib.upper_value - s_calib_blob->brake_calib.lower_value;

  int32_t range_throttle =
      s_calib_blob->throttle_calib.upper_value - s_calib_blob->throttle_calib.lower_value;
  
  int32_t brake_lower_value = (int32_t) s_calib_blob->brake_calib.lower_value; 
  int16_t reading_upscaled_brake =
      ((int16_t)*reading - s_calib_blob->brake_calib.lower_value) * EE_PEDAL_VALUE_DENOMINATOR;
  
  int32_t throttle_lower_value = (int32_t) s_calib_blob->throttle_calib.lower_value;
  int16_t reading_upscaled_throttle =
      ((int16_t)*reading - s_calib_blob->throttle_calib.lower_value) * EE_PEDAL_VALUE_DENOMINATOR;

  reading_upscaled_brake *= 100;
  reading_upscaled_throttle *= 100;

  if (channel == BRAKE_CHANNEL) {
    if (range_brake != 0) {
      reading_upscaled_brake /= (range_brake * EE_PEDAL_VALUE_DENOMINATOR);
      // Todo(Bafran): Make sure the data type is good with MCI
      *reading = prv_get_uint32(reading_upscaled_brake / 100.0);
    }
  } else if (channel == THROTTLE_CHANNEL) {
    if (range_throttle != 0) {
      reading_upscaled_throttle /= (range_throttle / EE_PEDAL_VALUE_DENOMINATOR);
      // Todo(Bafran): Make sure the data type is good with MCI
      *reading = prv_get_uint32(reading_upscaled_throttle / 100.0);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode read_throttle_data(uint32_t *reading) {
  return read_pedal_data(reading, THROTTLE_CHANNEL);
}

StatusCode read_brake_data(uint32_t *reading) {
  return read_pedal_data(reading, BRAKE_CHANNEL);
}
