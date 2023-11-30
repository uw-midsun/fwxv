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
#include "test_helpers.h"
#include "unity.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2

static Max11600Storage s_max11600_storage;
PedalCalibBlob global_calib_blob;
static PedalCalibrationStorage s_throttle_calibration_storage;
static PedalCalibrationStorage s_brake_calibration_storage;
static PersistStorage s_persist;
PedalCalibBlob *blob;

// I2CSettings i2c_settings = {
//   .speed = I2C_SPEED_FAST,
//   .scl = { .port = GPIO_PORT_B, .pin = 10 },
//   .sda = { .port = GPIO_PORT_B, .pin = 11 },
// };


/* TODO - ads1015 storage needs to be changed to MAX11600 (pending driver completion) */
void test_throttle_calibration_run(void) {
//   LOG_DEBUG("Please ensure the throttle is not being pressed.\n");
//   delay_s(7);
//   LOG_DEBUG("Beginning sampling\n");
//   pedal_calib_sample(&s_max11600_storage, &s_throttle_calibration_storage,
//                      &global_calib_blob.throttle_calib, THROTTLE_CHANNEL, PEDAL_UNPRESSED);
//
  //  PedalCalibrationStorage *calib_storage = &s_throttle_calibration_storage;
  //  PedalCalibrationData *data = &global_calib_blob.throttle_calib;
  //  calib_storage->sample_counter = 0;
  //  calib_storage->min_reading = INT16_MAX;
  //  calib_storage->max_reading = INT16_MIN;
  //  calib_storage->sample_counter++;
  //  calib_storage->min_reading = MIN(calib_storage->min_reading, 1);
  //  calib_storage->max_reading = MAX(calib_storage->min_reading, 1);
  //  data->upper_value = 0 / NUM_SAMPLES;
  //  data->lower_value = 0 / NUM_SAMPLES;
   LOG_DEBUG("Completed fake sampling\n");


   LOG_DEBUG("Completed unpressed sampling\n");
//   LOG_DEBUG("Please press and hold the throttle\n");
//   delay_s(7);
//   LOG_DEBUG("Beginning sampling\n");
//   pedal_calib_sample(&s_max11600_storage, &s_throttle_calibration_storage,
//                      &global_calib_blob.throttle_calib, THROTTLE_CHANNEL, PEDAL_PRESSED);
//
  
   LOG_DEBUG("Completed pressed sampling\n");

   calib_commit();
}

/* TODO - ads1015 storage needs to be changed to MAX11600 (pending driver completion) */
// void test_brake_calibration_run(void) {
//   LOG_DEBUG("Please ensure the brake is not being pressed.\n");
//   delay_s(7);
//   LOG_DEBUG("Beginning sampling\n");
//   pedal_calib_sample(&s_max11600_storage, &s_brake_calibration_storage,
//                      &global_calib_blob.brake_calib, BRAKE_CHANNEL, PEDAL_UNPRESSED);
//   LOG_DEBUG("Completed sampling\n");
//   LOG_DEBUG("Please press and hold the brake\n");
//   delay_s(7);
//   LOG_DEBUG("Beginning sampling\n");
//   pedal_calib_sample(&s_max11600_storage, &s_brake_calibration_storage,
//                      &global_calib_blob.brake_calib, BRAKE_CHANNEL, PEDAL_PRESSED);
//   LOG_DEBUG("Completed sampling\n");

//   calib_commit();
// }

int main(void) {
  log_init();
  gpio_init();
  interrupt_init();
  gpio_it_init();
  flash_init();

//   i2c_init(I2C_PORT_2, &i2c_settings);
//   max11600_init(&s_max11600_storage, I2C_PORT_2);

//   StatusCode ret = calib_init(&global_calib_blob, sizeof(global_calib_blob), true);
//   if (ret == STATUS_CODE_OK) {
//       LOG_DEBUG("calib_init test: OK\n");
//   } else {
//       LOG_DEBUG("calib_init test: FAILED (Error code: %d)\n", (int)ret);
//   }


//LOG_DEBUG("read value: %d\n" A);
// test_throttle_calibration_run();
//   test_brake_calibration_run();

//   pedal_calib_init(&s_throttle_calibration_storage);
//   pedal_calib_init(&s_brake_calibration_storage);

  
  LOG_DEBUG("READING");
  persist_init(&s_persist, CALIB_FLASH_PAGE, blob, sizeof(global_calib_blob), true);

  LOG_DEBUG("READING TWO");
  calib_init(&blob, sizeof(blob), true);

  LOG_DEBUG("READ FROM FLASHMEM %d\n", blob->brake_calib.lower_value);

  blob->brake_calib.upper_value = 182;
  
  persist_commit(&s_persist);
  
  

  while(1);
}