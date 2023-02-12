#include "pedal_calib.h"

#include <stdio.h>

#include "adc.h"
#include "ads1015.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_setters.h"

/* TODO - ads1015 storage needs to be changed to MAX11600 (pending driver completion) */
static Ads1015Storage *s_ads1015_storage;
void prv_callback_channel(Ads1015Channel channel, void *context) {
  int16_t reading = 0;
  PedalCalibrationStorage *storage = context;
  ads1015_read_raw(s_ads1015_storage, channel, &reading);
  storage->max_reading = MAX(storage->max_reading, reading);
  storage->min_reading = MIN(storage->min_reading, reading);
}

StatusCode pedal_calib_init(PedalCalibrationStorage *storage) {
  memset(storage, 0, sizeof(*storage));
  return STATUS_CODE_OK;
}
StatusCode pedal_calib_sample(Ads1015Storage *ads1015_storage, PedalCalibrationStorage *storage,
                              PedalCalibrationData *data, Ads1015Channel channel,
                              PedalState state) {
  s_ads1015_storage = ads1015_storage;
  ads1015_configure_channel(s_ads1015_storage, channel, false, NULL, NULL);
  ads1015_configure_channel(s_ads1015_storage, channel, true, prv_callback_channel, storage);
  if (state == PEDAL_PRESSED) {
    data->upper_value = storage->max_reading;
  } else {
    data->lower_value = storage->min_reading;
  }
  return STATUS_CODE_OK;
}
