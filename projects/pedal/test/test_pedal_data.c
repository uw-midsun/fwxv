#include <stdint.h>

#include "i2c.h"
#include "log.h"
#include "max11600.h"
#include "pedal_calib.h"
#include "pedal_data.h"
#include "pedal_setters.h"
#include "pedal_shared_resources_provider.h"
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
#define MOCK_THROTTLE_VALUE (500)
#define MOCK_BRAKE_VALUE (600)

// Expected values for pedal readings
#define EXPECTED_THROTTLE_READING_UPSCALED \
  ((MOCK_THROTTLE_VALUE - MOCK_THROTTLE_CALIB_LOWER) * EE_PEDAL_VALUE_DENOMINATOR * 100)

#define EXPECTED_BRAKE_READING_UPSCALED \
  ((MOCK_BRAKE_VALUE - MOCK_BRAKE_CALIB_LOWER) * EE_PEDAL_VALUE_DENOMINATOR * 100)

#define EXPECTED_THROTTLE_VALUE                                       \
  (((EXPECTED_THROTTLE_READING_UPSCALED /                             \
     (float)(MOCK_THROTTLE_CALIB_UPPER - MOCK_THROTTLE_CALIB_LOWER) * \
     EE_PEDAL_VALUE_DENOMINATOR) *                                    \
    UINT32_MAX))

#define EXPECTED_BRAKE_VALUE                                                                      \
  (((EXPECTED_BRAKE_READING_UPSCALED / (float)(MOCK_BRAKE_CALIB_UPPER - MOCK_BRAKE_CALIB_LOWER) * \
     EE_PEDAL_VALUE_DENOMINATOR) *                                                                \
    UINT32_MAX))

PedalCalibBlob s_calib_blob_instance = {
  .throttle_calib = { .lower_value = MOCK_THROTTLE_CALIB_LOWER,
                      .upper_value = MOCK_THROTTLE_CALIB_UPPER },
  .brake_calib = { .lower_value = MOCK_BRAKE_CALIB_LOWER, .upper_value = MOCK_BRAKE_CALIB_UPPER },
};

PedalCalibBlob *s_calib_blob_mock = &s_calib_blob_instance;
Max11600Storage max11600_storage_mock;

// Mock function for max11600_read_raw
StatusCode TEST_MOCK(max11600_read_raw)(Max11600Storage *storage) {
  storage->channel_readings[THROTTLE_CHANNEL] = MOCK_THROTTLE_VALUE;  // Mock throttle value
  storage->channel_readings[BRAKE_CHANNEL] = MOCK_BRAKE_VALUE;        // Mock brake value
  return STATUS_CODE_OK;
}

void setup_test(void) {
  log_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };

  i2c_init(I2C_PORT_2, &i2c_settings);
  max11600_init(&max11600_storage_mock, I2C_PORT_2);
}

Max11600Storage *s_max11600_storage_mock = &max11600_storage_mock;

void teardown_test(void) {}

static StatusCode read_pedal_data(uint32_t *reading, MAX11600Channel channel) {
  status_ok_or_return(max11600_read_raw(s_max11600_storage_mock));
  int32_t range;
  int32_t lower_value;

  if (channel == THROTTLE_CHANNEL) {
    range = s_calib_blob_mock->throttle_calib.upper_value -
            s_calib_blob_mock->throttle_calib.lower_value;
    lower_value = s_calib_blob_mock->throttle_calib.lower_value;
  } else if (channel == BRAKE_CHANNEL) {
    range = s_calib_blob_mock->brake_calib.upper_value - s_calib_blob_mock->brake_calib.lower_value;
    lower_value = s_calib_blob_mock->brake_calib.lower_value;
  } else {
    // Return an error status code if an unexpected channel is used
    return STATUS_CODE_INVALID_ARGS;
  }

  int32_t reading_upscaled =
      (int32_t)s_max11600_storage_mock->channel_readings[channel] * EE_PEDAL_VALUE_DENOMINATOR;
  reading_upscaled -= lower_value * EE_PEDAL_VALUE_DENOMINATOR;
  reading_upscaled *= 100;

  if (range != 0) {
    reading_upscaled /= (range * EE_PEDAL_VALUE_DENOMINATOR);
    *reading = (uint32_t)((reading_upscaled / 100.0) * UINT32_MAX);
  }
  return STATUS_CODE_OK;
}

void test_pedal_cycle(void) {
  uint32_t throttle_value = 100;
  uint32_t brake_value = 100;
  uint32_t *throttle_reading = &throttle_value;
  uint32_t *brake_reading = &brake_value;

  // Read throttle and brake values
  StatusCode status_throttle = read_pedal_data(throttle_reading, THROTTLE_CHANNEL);
  StatusCode status_brake = read_pedal_data(brake_reading, BRAKE_CHANNEL);

  TEST_ASSERT_OK(status_throttle);
  TEST_ASSERT_OK(status_brake);

  // Use CAN setters to set throttle and brake values
  set_pedal_output_throttle_output(*throttle_reading);
  set_pedal_output_brake_output(*brake_reading);

  // Print the values after setting
  printf("\n\nExpected throttle value: %f\n", EXPECTED_THROTTLE_VALUE);
  printf("Expected brake value: %f\n", EXPECTED_BRAKE_VALUE);

  printf("\nSet throttle value: %d\n", g_tx_struct.pedal_output_throttle_output);
  printf("Set brake value: %d\n", g_tx_struct.pedal_output_brake_output);

  // Transmit the data over CAN
  can_tx_all();

  // Check the correct values for the throttle and brake signals are in the g_tx_struct
  TEST_ASSERT_EQUAL(EXPECTED_THROTTLE_VALUE, g_tx_struct.pedal_output_throttle_output);
  TEST_ASSERT_EQUAL(EXPECTED_BRAKE_VALUE, g_tx_struct.pedal_output_brake_output);
}
