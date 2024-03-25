#include <stdint.h>

#include "i2c.h"
#include "log.h"
#include "pedal.h"
#include "pedal_calib.h"
#include "pedal_setters.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

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
  calib_init(&s_mock_calib_blob, sizeof(s_mock_calib_blob), false);
  pedal_init(&s_mock_calib_blob);
}

void teardown_test(void) {}

void test_pedal_cycle(void) {
  pedal_run();

  // Check the correct values for the throttle and brake signals are in the g_tx_struct
  TEST_ASSERT_EQUAL(EXPECTED_THROTTLE_VALUE,
                    prv_get_float(g_tx_struct.pedal_output_throttle_output));
  TEST_ASSERT_EQUAL(EXPECTED_BRAKE_VALUE, prv_get_float(g_tx_struct.pedal_output_brake_output));
}
