#include <stdint.h>
#include "max11600.h"
#include "can_hw.h"
#include "can_msg.h"
#include "can.h"
#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "pedal_data.h"
#include "pedal_calib.h"
#include "soft_timer.h"
#include "unity.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2
#define EE_PEDAL_VALUE_DENOMINATOR (1 << 12)

extern g_tx_struct;

//Mock function for pedal calibration
PedalCalibBlob global_calib_blob_mock = {
  .throttle_calib = { .lower_value = 0, .upper_value = 1000 },
  .brake_calib = { .lower_value = 0, .upper_value = 1000 },
};

//Mock function for max11600_read_converted
StatusCode TEST_MOCK(max11600_read_converted)(Max11600Storage *storage) {
  storage->channel_readings[THROTTLE_CHANNEL] = 500; //Mock throttle value
  storage->channel_readings[BRAKE_CHANNEL] = 600;    //Mock brake value
  return STATUS_CODE_OK;
}

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

StatusCode read_pedal_data(int16_t *reading, MAX11600Channel channel) {
  status_ok_or_return(max11600_read_converted(&s_max11600_storage));
  int32_t range = channel == THROTTLE_CHANNEL
                      ? global_calib_blob_mock.throttle_calib.upper_value - global_calib_blob_mock.throttle_calib.lower_value
                      : global_calib_blob_mock.brake_calib.upper_value - global_calib_blob_mock.brake_calib.lower_value;

  *reading = (int16_t)s_max11600_storage.channel_readings[channel];
  int32_t reading_upscaled = (int32_t)*reading * EE_PEDAL_VALUE_DENOMINATOR;
  reading_upscaled *= 100;

  if (range != 0) {
    reading_upscaled /= (range * EE_PEDAL_VALUE_DENOMINATOR);
    *reading = (int16_t)reading_upscaled;
  }
  return STATUS_CODE_OK;
}

void test_pedal_cycle(void) {
  int16_t throttle_reading, brake_reading;

  //Read throttle and brake values
  StatusCode status_throttle = read_pedal_data(&throttle_reading, THROTTLE_CHANNEL);
  StatusCode status_brake = read_pedal_data(&brake_reading, BRAKE_CHANNEL);

  TEST_ASSERT_OK(status_throttle);
  TEST_ASSERT_OK(status_brake);

  //Use CAN setters to set throttle and brake values
  set_pedal_output_throttle_output(throttle_reading);
  set_pedal_output_brake_output(brake_reading);

  //Transmit the data over CAN
  can_tx_all();

  //Check the correct values for the throttle and brake signals are in the g_tx_struct
  int32_t throttle_sent = (int32_t)(g_tx_struct.pedal_output.throttle_output);
  int32_t brake_sent = (int32_t)(g_tx_struct.pedal_output.brake_output);
  TEST_ASSERT_EQUAL_INT32(throttle_reading, throttle_sent);
  TEST_ASSERT_EQUAL_INT32(brake_reading, brake_sent);
}
