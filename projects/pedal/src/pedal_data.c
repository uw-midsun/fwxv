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

StatusCode read_throttle_data(uint32_t *reading) {
  volatile uint16_t adc_reading;
  adc_read_raw(throttle, &adc_reading);

  // Convert ADC Reading to readable voltage by normalizing with calibration data and dividing
  // to get percentage press. Brake is now just a GPIO. Negatives and > 100 values will be
  // capped.
  int16_t range_throttle =
      s_calib_blob->throttle_calib.upper_value - s_calib_blob->throttle_calib.lower_value;
  volatile float calculated_reading =
      (((float)adc_reading - (float)s_calib_blob->throttle_calib.lower_value) / range_throttle);
  // Readings are inverted
  calculated_reading = 1 - calculated_reading;

  if (calculated_reading < 0) {
    calculated_reading = 0;
  } else if (calculated_reading > 1) {
    calculated_reading = 1;
  }
  memcpy(reading, &calculated_reading, sizeof(calculated_reading));

  return STATUS_CODE_OK;
}
