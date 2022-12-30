#include "ads1015.h"
#include "calib.h"
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_calib.h"
#include "test_helpers.h"
#include "unity.h"

#define THROTTLE_CHANNEL ADS1015_CHANNEL_0
#define BRAKE_CHANNEL ADS1015_CHANNEL_2

static Ads1015Storage s_ads1015_storage;
static PedalCalibBlob s_calib_blob;
static PedalCalibrationStorage s_throttle_calibration_storage;
static PedalCalibrationStorage s_brake_calibration_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  flash_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };
  ads1015_init(&s_ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  TEST_ASSERT_OK(calib_init(&s_calib_blob, sizeof(s_calib_blob), true));
  pedal_calib_init(&s_throttle_calibration_storage);
  pedal_calib_init(&s_brake_calibration_storage);
}

void teardown_test(void) {}

void test_throttle_calibration_run(void) {
  LOG_DEBUG("Please ensure the throttle is not being pressed.\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_ads1015_storage, &s_throttle_calibration_storage,
                     &s_calib_blob.throttle_calib, THROTTLE_CHANNEL, PEDAL_UNPRESSED);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please press and hold the throttle\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_ads1015_storage, &s_throttle_calibration_storage,
                     &s_calib_blob.throttle_calib, THROTTLE_CHANNEL, PEDAL_PRESSED);
  LOG_DEBUG("Completed sampling\n");

  calib_commit();
}

void test_brake_calibration_run(void) {
  LOG_DEBUG("Please ensure the brake is not being pressed.\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_ads1015_storage, &s_brake_calibration_storage, &s_calib_blob.brake_calib,
                     BRAKE_CHANNEL, PEDAL_UNPRESSED);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please press and hold the brake\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  pedal_calib_sample(&s_ads1015_storage, &s_brake_calibration_storage, &s_calib_blob.brake_calib,
                     BRAKE_CHANNEL, PEDAL_PRESSED);
  LOG_DEBUG("Completed sampling\n");

  calib_commit();
}
