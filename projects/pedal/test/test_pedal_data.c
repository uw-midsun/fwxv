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
#define MOCK_THROTTLE_CALIB_UPPER 250
#define MOCK_THROTTLE_CALIB_LOWER 5
#define MOCK_BRAKE_CALIB_UPPER 250
#define MOCK_BRAKE_CALIB_LOWER 5

// Definitions for mock pedal values
#define MOCK_THROTTLE_VALUE (250)
#define MOCK_BRAKE_VALUE (5)

// Expected values for pedal readings
#define EXPECTED_THROTTLE_VALUE 1.0
#define EXPECTED_BRAKE_VALUE 0

PedalCalibBlob s_mock_calib_blob = {
  .throttle_calib = { .lower_value = MOCK_THROTTLE_CALIB_LOWER,
                      .upper_value = MOCK_THROTTLE_CALIB_UPPER },
  .brake_calib = { .lower_value = MOCK_BRAKE_CALIB_LOWER, .upper_value = MOCK_BRAKE_CALIB_UPPER },
};

Max11600Storage max11600_storage_instance;
Max11600Storage *max11600_storage_mock = &max11600_storage_instance;

// Mock function for max11600_read_raw
StatusCode TEST_MOCK(max11600_read_raw)(Max11600Storage *storage) {
  storage->channel_readings[THROTTLE_CHANNEL] = MOCK_THROTTLE_VALUE;  // Mock throttle value
  storage->channel_readings[BRAKE_CHANNEL] = MOCK_BRAKE_VALUE;        // Mock brake value
  return STATUS_CODE_OK;
}

// Taken from MCI project to check against the values it needs to receive
static float prv_get_float(uint32_t u) {
  union {
    float f;
    uint32_t u;
  } fu = { .u = u };
  return fu.f;
}

void setup_test(void) {
  log_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };

  i2c_init(I2C_PORT_2, &i2c_settings);
  max11600_init(max11600_storage_mock, I2C_PORT_2);
  pedal_resources_init(max11600_storage_mock, &s_mock_calib_blob);
  pedal_data_init(max11600_storage_mock, &s_mock_calib_blob);
}

void teardown_test(void) {}

void test_pedal_cycle(void) {
  int32_t throttle_reading = 0;
  int32_t brake_reading = 0;

  // Read throttle and brake values
  read_throttle_data(&throttle_reading);
  read_brake_data(&brake_reading);

  // Use CAN setters to set throttle and brake values
  set_pedal_output_throttle_output((uint32_t)throttle_reading);
  set_pedal_output_brake_output((uint32_t)brake_reading);

  // Check the correct values for the throttle and brake signals are in the g_tx_struct
  TEST_ASSERT_EQUAL(EXPECTED_THROTTLE_VALUE,
                    prv_get_float(g_tx_struct.pedal_output_throttle_output));
  TEST_ASSERT_EQUAL(EXPECTED_BRAKE_VALUE, prv_get_float(g_tx_struct.pedal_output_brake_output));
}
