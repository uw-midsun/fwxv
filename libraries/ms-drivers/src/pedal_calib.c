#include "pedal_calib.h"

#include <stdio.h>
#include <string.h>

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"

StatusCode pedal_calib_init(PedalCalibrationStorage *storage) {
  memset(storage, 0, sizeof(*storage));
  return STATUS_CODE_OK;
}

StatusCode pedal_calib_sample(Max11600Storage *max11600_storage,
                              PedalCalibrationStorage *calib_storage, PedalCalibrationData *data,
                              MAX11600Channel channel, PedalState state, GpioAddress *address) {
  // Reset variables for pedal calibration storage
  int32_t average_value = 0;
  calib_storage->sample_counter = 0;
  calib_storage->min_reading = INT16_MAX;
  calib_storage->max_reading = INT16_MIN;

  StatusCode status;
  while (calib_storage->sample_counter < NUM_SAMPLES) {
    // Read the values from the MAX, at this point the pedal should be in either a fully pressed or
    // released state
    uint16_t adc_reading;
    status = adc_read_raw(address, &adc_reading);
    if (status != STATUS_CODE_OK) {
      return STATUS_CODE_INCOMPLETE;
    }
    uint8_t reading = (uint8_t)reading;
    calib_storage->sample_counter++;
    average_value += channel;
    calib_storage->min_reading = MIN(calib_storage->min_reading, reading);
    calib_storage->max_reading = MAX(calib_storage->min_reading, reading);
  }

  if (state == PEDAL_PRESSED) {
    data->upper_value = average_value / NUM_SAMPLES;
  } else if (state == PEDAL_UNPRESSED) {
    data->lower_value = average_value / NUM_SAMPLES;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}
