#include "calib.h"
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "max11600.h"
#include "pedal_calib.h"
#include "pedal_data.h"
#include "test_helpers.h"
#include "unity.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2

static const GpioAddress brake = ADC_POT1_BRAKE;
static const GpioAddress throttle = ADC_HALL_SENSOR;

static Max11600Storage s_max11600_storage;
PedalCalibBlob global_calib_blob;
static PedalCalibrationStorage s_throttle_calibration_storage;
static PedalCalibrationStorage s_brake_calibration_storage;

/* TODO - ads1015 storage needs to be changed to MAX11600 (pending driver completion) */
void test_throttle_calibration_run(void) {
  LOG_DEBUG("Please ensure the throttle is not being pressed.\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_max11600_storage, &s_throttle_calibration_storage,
                     &global_calib_blob.throttle_calib, THROTTLE_CHANNEL, PEDAL_UNPRESSED,
                     &throttle);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please press and hold the throttle\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_max11600_storage, &s_throttle_calibration_storage,
                     &global_calib_blob.throttle_calib, THROTTLE_CHANNEL, PEDAL_PRESSED, &throttle);
  LOG_DEBUG("Completed sampling\n");

  calib_commit();
}

/* TODO - ads1015 storage needs to be changed to MAX11600 (pending driver completion) */
void test_brake_calibration_run(void) {
  LOG_DEBUG("Please ensure the brake is not being pressed.\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_max11600_storage, &s_brake_calibration_storage,
                     &global_calib_blob.brake_calib, BRAKE_CHANNEL, PEDAL_UNPRESSED, &brake);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please press and hold the brake\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_max11600_storage, &s_brake_calibration_storage,
                     &global_calib_blob.brake_calib, BRAKE_CHANNEL, PEDAL_PRESSED, &brake);
  LOG_DEBUG("Completed sampling\n");

  calib_commit();
}

int main(void) {
  log_init();
  gpio_init();
  interrupt_init();
  gpio_it_init();
  flash_init();

  StatusCode ret = calib_init(&global_calib_blob, sizeof(global_calib_blob), true);
  if (ret == STATUS_CODE_OK) {
    LOG_DEBUG("calib_init test: OK\n");
  } else {
    LOG_DEBUG("calib_init test: FAILED (Error code: %d)\n", (int)ret);
  }

  pedal_calib_init(&s_throttle_calibration_storage);
  pedal_calib_init(&s_brake_calibration_storage);

  test_throttle_calibration_run();
  test_brake_calibration_run();

  while (1);
}
