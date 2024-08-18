#include "mppt.h"
#include "power_sense.h"
#include "task_test_helpers.h"
#include "unity.h"

MPPTData mock_mppt = { .algorithm = MPPT_INCREMENTAL_CONDUCTANCE };

void setup_test(void) {
  mppt_init(&mock_mppt);
}

void teardown_test(void) {}

void test_mppt_init(void) {
  mppt_init(&mock_mppt);
  TEST_ASSERT_EQUAL_UINT32(0, mock_mppt.power);
  TEST_ASSERT_EQUAL_UINT32(0, mock_mppt.prev_power);
  TEST_ASSERT_EQUAL_UINT32(0, mock_mppt.voltage);
  TEST_ASSERT_EQUAL_UINT32(0, mock_mppt.prev_voltage);
  TEST_ASSERT_EQUAL_UINT32(0, mock_mppt.current);
  TEST_ASSERT_EQUAL_UINT32(0, mock_mppt.prev_current);
  TEST_ASSERT_EQUAL_UINT16(0, mock_mppt.pwm_dc);
  TEST_ASSERT_EQUAL(MPPT_INCREMENTAL_CONDUCTANCE, mock_mppt.algorithm);
}

void test_ic_power_decrease_voltage_decrease(void) {}
