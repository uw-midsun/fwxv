#include "mppt.h"
#include "power_sense.h"
#include "task_test_helpers.h"
#include "unity.h"

MPPTData mock_mppt = { .algorithm = MPPT_PERTURB_OBSERVE };

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
  TEST_ASSERT_EQUAL(MPPT_PERTURB_OBSERVE, mock_mppt.algorithm);
}

void preturb_power_increase_voltage_increase(void) {
  mock_mppt.prev_power = 10;
  mock_mppt.power = 20;
  mock_mppt.prev_voltage = 5;
  mock_mppt.voltage = 10;
  mock_mppt.pwm_dc = 50;

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(50 + DUTY_CYCLE_STEP, mock_mppt.pwm_dc);
}

void preturb_power_increase_voltage_decrease(void) {
  mock_mppt.prev_power = 10;
  mock_mppt.power = 20;
  mock_mppt.prev_voltage = 10;
  mock_mppt.voltage = 5;
  mock_mppt.pwm_dc = 50;

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(50 - DUTY_CYCLE_STEP, mock_mppt.pwm_dc);
}

void test_mppt_set_algorithm(void) {
  mppt_set_algorithm(MPPT_PERTURB_OBSERVE);
  TEST_ASSERT_EQUAL(MPPT_PERTURB_OBSERVE, mock_mppt.algorithm);
}

void test_mppt_set_pwm(void) {
  mppt_set_pwm(75);
  TEST_ASSERT_EQUAL_UINT16(75, mock_mppt.pwm_dc);
}

void test_mppt_stop(void) {
  mppt_stop();
  TEST_ASSERT_EQUAL_UINT16(0, mock_mppt.pwm_dc);
  TEST_ASSERT_EQUAL(MPPT_STOPPED, mock_mppt.algorithm);
}
