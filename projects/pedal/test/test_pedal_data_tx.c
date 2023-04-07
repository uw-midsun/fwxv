#include <stdint.h>

#include "log.h"
#include "max11600.h"
#include "pedal_calib.h"
#include "pedal_data.h"
#include "pedal_setters.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define THROTTLE_CHANNEL MAX11600_CHANNEL_0
#define BRAKE_CHANNEL MAX11600_CHANNEL_2
#define EE_PEDAL_VALUE_DENOMINATOR (1 << 12)

// Definitions for mock pedal calibrations
#define MOCK_THROTTLE_CALIB_LOWER 0
#define MOCK_THROTTLE_CALIB_UPPER 1000
#define MOCK_BRAKE_CALIB_LOWER 0
#define MOCK_BRAKE_CALIB_UPPER 1000

// Definitions for mock pedal values
#define MOCK_THROTTLE_VALUE ((int32_t)500)
#define MOCK_BRAKE_VALUE ((int32_t)600)

// Expected values for pedal readings
#define EXPECTED_THROTTLE_VALUE                                 \
  (((MOCK_THROTTLE_VALUE * EE_PEDAL_VALUE_DENOMINATOR) * 100) / \
   ((MOCK_THROTTLE_CALIB_UPPER - MOCK_THROTTLE_CALIB_LOWER) * EE_PEDAL_VALUE_DENOMINATOR))
#define EXPECTED_BRAKE_VALUE                                 \
  (((MOCK_BRAKE_VALUE * EE_PEDAL_VALUE_DENOMINATOR) * 100) / \
   ((MOCK_BRAKE_CALIB_UPPER - MOCK_BRAKE_CALIB_LOWER) * EE_PEDAL_VALUE_DENOMINATOR))

// Mock function for pedal calibration
PedalCalibBlob global_calib_blob_mock = {
  .throttle_calib = { .lower_value = MOCK_THROTTLE_CALIB_LOWER,
                      .upper_value = MOCK_THROTTLE_CALIB_UPPER },
  .brake_calib = { .lower_value = MOCK_BRAKE_CALIB_LOWER, .upper_value = MOCK_BRAKE_CALIB_UPPER },
};

// Mock function for max11600_read_converted
StatusCode TEST_MOCK(max11600_read_converted)(Max11600Storage *storage) {
  storage->channel_readings[THROTTLE_CHANNEL] = MOCK_THROTTLE_VALUE;  // Mock throttle value
  storage->channel_readings[BRAKE_CHANNEL] = MOCK_BRAKE_VALUE;        // Mock brake value
  return STATUS_CODE_OK;
}

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

static StatusCode read_pedal_data(int16_t *reading, MAX11600Channel channel) {
  status_ok_or_return(max11600_read_converted(&s_max11600_storage));
  int32_t range = channel == THROTTLE_CHANNEL
                      ? global_calib_blob_mock.throttle_calib.upper_value -
                            global_calib_blob_mock.throttle_calib.lower_value
                      : global_calib_blob_mock.brake_calib.upper_value -
                            global_calib_blob_mock.brake_calib.lower_value;

  *reading = (int16_t)s_max11600_storage.channel_readings[channel];
  int32_t reading_upscaled = (int32_t)*reading * EE_PEDAL_VALUE_DENOMINATOR;

  if (range != 0) {
    reading_upscaled = (reading_upscaled * 100) / (range * EE_PEDAL_VALUE_DENOMINATOR);
    *reading = (int16_t)reading_upscaled;
  }
  return STATUS_CODE_OK;
}

void test_pedal_cycle(void) {
  int16_t throttle_value = 100;
  int16_t brake_value = 100;
  int16_t *throttle_reading = &throttle_value;
  int16_t *brake_reading = &brake_value;

  // Read throttle and brake values
  StatusCode status_throttle = read_pedal_data(throttle_reading, THROTTLE_CHANNEL);
  StatusCode status_brake = read_pedal_data(brake_reading, BRAKE_CHANNEL);

  TEST_ASSERT_OK(status_throttle);
  TEST_ASSERT_OK(status_brake);

  // Use CAN setters to set throttle and brake values
  set_pedal_output_throttle_output((uint16_t)(*throttle_reading));
  set_pedal_output_brake_output((uint16_t)(*brake_reading));

  // Transmit the data over CAN
  can_tx_all();

  // Check the correct values for the throttle and brake signals are in the g_tx_struct
  TEST_ASSERT_EQUAL_INT32(EXPECTED_THROTTLE_VALUE, g_tx_struct.pedal_output_throttle_output);
  TEST_ASSERT_EQUAL_INT32(EXPECTED_BRAKE_VALUE, g_tx_struct.pedal_output_brake_output);
}