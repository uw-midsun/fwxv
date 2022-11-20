#include "pedal_calib.h"

#include <stdio.h>

#include "adc.h"
#include "ads1015.h"
#include "can.h"
#include "can_board_ids.h"
#include "can_msg.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
// #include "main.c"
#include "pedal_setters.h"
#include "soft_timer.h"
#include "tasks.h"

// TODO(Scrubpai): Read the ADC when throttle and brake are pressed down or released,
// and use that reading to set the upper (max) and lower bounds (min) of each pedal

static Ads1015Storage *s_ads1015_storage_throttle;
void prv_callback_channel_throttle(Ads1015Channel channel, void *context) {
  int16_t reading = 0;
  PedalCalibrationStorage *storage = context;
  ads1015_read_raw(s_ads1015_storage_throttle, channel, &reading);
  storage->max_reading = MAX(storage->max_reading, reading);
  storage->min_reading = MIN(storage->min_reading, reading);
}
static Ads1015Storage *s_ads1015_storage_brake;
void prv_callback_channel_brake(Ads1015Channel channel, void *context) {
  int16_t reading = 0;
  PedalCalibrationStorage *storage = context;
  ads1015_read_raw(s_ads1015_storage_brake, channel, &reading);
  storage->max_reading = MAX(storage->max_reading, reading);
  storage->min_reading = MIN(storage->min_reading, reading);
}
void pedal_calibrate() {
  if (read_pedal_throttle || !read_pedal_throttle) {
    // Read ADC and update min/max on blob
    StatusCode pedal_calib_init(PedalCalibrationStorage * storage) {
      return STATUS_CODE_OK;
    }
    StatusCode pedal_calib_write(
        Ads1015Storage * ads1015_storage, PedalCalibrationStorage * storage,
        PedalCalibrationData * throttle_calib, Ads1015Channel channel, PedalCalibBlob * blob) {
      s_ads1015_storage_throttle = ads1015_storage;
      // Disables channel
      ads1015_configure_channel(s_ads1015_storage_throttle, channel, false, NULL, NULL);
      ads1015_configure_channel(s_ads1015_storage_throttle, channel, true,
                                prv_callback_channel_throttle, storage);
      if (read_pedal_throttle) {
        blob->throttle_calib.upper_value =
            MAX(blob->throttle_calib.upper_value, storage->max_reading);
      } else {
        blob->throttle_calib.lower_value =
            MIN(blob->throttle_calib.lower_value, storage->min_reading);
      }
      return STATUS_CODE_OK;
    }
  }
  if (read_pedal_brake || !read_pedal_brake) {
    // Read ADC and update min/max on board
    StatusCode pedal_calib_init(PedalCalibrationStorage * storage) {
      return STATUS_CODE_OK;
    }
    StatusCode pedal_calib_write(
        Ads1015Storage * ads1015_storage, PedalCalibrationStorage * storage,
        PedalCalibrationData * brake_calib, Ads1015Channel channel, PedalCalibBlob * blob) {
      s_ads1015_storage_brake = ads1015_storage;
      // Disables channel
      ads1015_configure_channel(s_ads1015_storage_brake, channel, false, NULL, NULL);

      ads1015_configure_channel(s_ads1015_storage_brake, channel, true, prv_callback_channel_brake,
                                storage);
      if (read_pedal_brake) {
        blob->brake_calib.upper_value = MAX(blob->brake_calib.upper_value, storage->max_reading);
      } else {
        blob->brake_calib.upper_value = MAX(blob->brake_calib.upper_value, storage->max_reading);
      }
      return STATUS_CODE_OK;
    }
  }
}
