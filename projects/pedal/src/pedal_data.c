#include "pedal_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "log.h"
#include "pedal_calib.h"
#include "pedal_shared_resources_provider.h"
#include "status.h"

static const GpioAddress throttle = ADC_HALL_SENSOR;

static PedalCalibBlob *s_calib_blob;

void pedal_data_init() {
  s_calib_blob = get_shared_pedal_calib_blob();
}

StatusCode read_throttle_data(int32_t *reading) {
  volatile uint16_t adc_reading;
  adc_read_raw(throttle, &adc_reading);

  memcpy(reading, &adc_reading, sizeof(adc_reading));
  // Convert ADC Reading to readable voltage by normalizing with calibration data and dividing
  // to get percentage press. Brake is now just a GPIO. Negatives and > 100 values will be
  // capped.
  int16_t range_throttle =
      s_calib_blob->throttle_calib.upper_value - s_calib_blob->throttle_calib.lower_value;
  volatile float calculated_reading =
      (((float)*reading - (float)s_calib_blob->throttle_calib.lower_value) / range_throttle) * 100;
  *reading = 100 - (int32_t)calculated_reading;

  if (*reading < 0) {
    *reading = 0;
  } else if (*reading > 100) {
    *reading = 100;
  }

  return STATUS_CODE_OK;
}
