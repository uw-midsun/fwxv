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

void test_power_increase_voltage_increase(void) {
  mock_mppt.prev_voltage = 5;
  mock_mppt.voltage = 10;
  mock_mppt.prev_current = 2;
  mock_mppt.current = 2;
  mock_mppt.pwm_dc = 500;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500 + DUTY_CYCLE_STEP, mock_mppt.pwm_dc);
}

void test_power_increase_voltage_decrease(void) {
  mock_mppt.prev_voltage = 10;
  mock_mppt.voltage = 5;
  mock_mppt.prev_current = 1;
  mock_mppt.current = 4;
  mock_mppt.pwm_dc = 500;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500 - DUTY_CYCLE_STEP, mock_mppt.pwm_dc);
}

void test_power_decrease_voltage_increase(void) {
  mock_mppt.prev_voltage = 5;
  mock_mppt.voltage = 10;
  mock_mppt.prev_current = 4;
  mock_mppt.current = 1;
  mock_mppt.pwm_dc = 500;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500 - DUTY_CYCLE_STEP, mock_mppt.pwm_dc);
}

void test_power_decrease_voltage_decrease(void) {
  mock_mppt.prev_voltage = 10;
  mock_mppt.voltage = 5;
  mock_mppt.prev_current = 2;
  mock_mppt.current = 2;
  mock_mppt.pwm_dc = 500;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500 + DUTY_CYCLE_STEP, mock_mppt.pwm_dc);
}

void test_no_change(void) {
  mock_mppt.prev_voltage = 10;
  mock_mppt.voltage = 10;
  mock_mppt.prev_current = 5;
  mock_mppt.current = 5;
  mock_mppt.pwm_dc = 500;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500, mock_mppt.pwm_dc);
}

void test_small_voltage_big_current(void) {
  mock_mppt.prev_voltage = 50;
  mock_mppt.voltage = 49;
  mock_mppt.prev_current = 19;
  mock_mppt.current = 21;
  mock_mppt.pwm_dc = 500;

  calculate_prev_power();
  calculate_power();
  mppt_run();

  // Power increased, voltage went down so we decrease voltage more
  TEST_ASSERT_EQUAL_UINT16(500 - DUTY_CYCLE_STEP, mock_mppt.pwm_dc);
}

void test_oscillating_power_and_voltage(void) {
  mock_mppt.prev_voltage = 50;
  mock_mppt.voltage = 60;
  mock_mppt.prev_current = 15;
  mock_mppt.current = 18;
  mock_mppt.pwm_dc = 500;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500 + DUTY_CYCLE_STEP, mock_mppt.pwm_dc);

  // Power increases with a voltage decrease -> PWM must continue to decrease (Lowers buck-boost
  // Vout)
  mock_mppt.prev_voltage = 60;
  mock_mppt.voltage = 50;
  mock_mppt.prev_current = 18;
  mock_mppt.current = 30;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500, mock_mppt.pwm_dc);

  // Power decreases with a voltage increases -> PWM must decrease (Lowers buck-boost Vout)
  mock_mppt.prev_voltage = 50;
  mock_mppt.voltage = 60;
  mock_mppt.prev_current = 15;
  mock_mppt.current = 10;

  calculate_prev_power();
  calculate_power();

  mppt_run();

  TEST_ASSERT_EQUAL_UINT16(500 - DUTY_CYCLE_STEP, mock_mppt.pwm_dc);

  // Power decreases with voltage decrease -> PWM must increase (Increases buck-boost Vout)
  mock_mppt.prev_voltage = 45;
  mock_mppt.voltage = 35;
  mock_mppt.prev_current = 17;
  mock_mppt.current = 15;

  calculate_prev_power();
  calculate_power();

  mppt_run();
  TEST_ASSERT_EQUAL_UINT16(500, mock_mppt.pwm_dc);
}
