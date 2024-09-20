#include "unity.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "state_of_charge.h"
#include "bms.h"
#include "log.h"
#include "tasks.h"
#include "delay.h"

static BmsStorage mock_bms_storage;
static TickType_t mock_time = 1;

// TODO Fix this, currently delaying for xTaskGetTickCount to work
TickType_t TEST_MOCK(xTaskGetTickCount)(void) {
    return mock_time;
}

void setup_test(void) {
  state_of_charge_init(&mock_bms_storage);
}

void teardown_test(void) {}

TEST_IN_TASK
void test_coulomb_counting_zero_current(void) {
  mock_bms_storage.current_storage.current = 0;
  mock_bms_storage.current_storage.voltage = 35000;
  set_averaged_soc(0.5f);

  coulomb_counting_soc();

  TEST_ASSERT_EQUAL_FLOAT(0.5f, get_i_soc());
}

TEST_IN_TASK
void test_coulomb_counting_constant_current(void) {
  mock_bms_storage.current_storage.current = -100000; // Pack is drawing 100 AMPS... I hope this never happens
  set_last_current(-100000);
  set_averaged_soc(0.5f);

  mock_time = 0;
  set_last_time(xTaskGetTickCount());
  mock_time = 60000; // 1 minute
  coulomb_counting_soc();

  float expected_soc = 0.5f + (0.5f * (-100000 - 100000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
  
  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());
  mock_time = 120000; // 2 minutes
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-100000 - 100000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
  
  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());
  mock_time = 180000; // 3 minutes
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-100000 - 100000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  LOG_DEBUG("Expected SOC %d\n", (int)(expected_soc* 100000.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
}

TEST_IN_TASK
void test_coulomb_counting_dynamic_current(void) {
  mock_bms_storage.current_storage.current = -200000; // Pack is discharging 200 AMPS
  set_last_current(-100000);
  set_averaged_soc(0.5f);

  mock_time = 0;
  set_last_time(xTaskGetTickCount());
  mock_time = 60000; // 1 minute
  coulomb_counting_soc();

  float expected_soc = 0.5f + (0.5f * (-100000 - 200000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
  
  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());

  set_last_current(-200000);
  mock_bms_storage.current_storage.current = -400000; // Pack is discharging 400 AMPS
  mock_time = 120000; // 2 minutes
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-200000 - 400000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
  
  set_averaged_soc(expected_soc);
  set_last_time(xTaskGetTickCount());

  set_last_current(-400000);
  mock_bms_storage.current_storage.current = 300000; // Pack is charging 300 AMPS
  mock_time = 180000; // 3 minutes
  coulomb_counting_soc();

  expected_soc = expected_soc + (0.5f * (-400000 + 300000) * (60.0f / 3600.0f)) / PACK_CAPACITY_MAH;
  LOG_DEBUG("Expected SOC %d\n", (int)(expected_soc* 100000.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
}

TEST_IN_TASK
void test_initial_state_of_charge(void) {
    mock_bms_storage.current_storage.current = 0;
    mock_bms_storage.current_storage.voltage = 3500;
    state_of_charge_init(&mock_bms_storage);

    // Check if the initial SOC is set based on the OCV voltage
    float expected_soc = 0; // Update based on your lookup logic
    TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_v_soc());
    TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_i_soc());
    TEST_ASSERT_FLOAT_WITHIN(0.00001f, expected_soc, get_averaged_soc());
}

TEST_IN_TASK
void test_ramp_voltage_weight_high_soc(void) {
    set_averaged_soc(80.0f);
    ramp_voltage_weight();
    TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.20f + ((0.6f / 30.0f) * (80.0f - 70.0f)), get_voltage_weight());
}

TEST_IN_TASK
void test_ramp_voltage_weight_low_soc(void) {
    set_averaged_soc(20.0f);
    ramp_voltage_weight();
    TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.20f + ((0.6f / 30.0f) * (30.0f - 20.0f)), get_voltage_weight());
}

TEST_IN_TASK
void test_ramp_voltage_weight_normal_soc(void) {
    set_averaged_soc(50.0f);
    ramp_voltage_weight();
    TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.20f, get_voltage_weight());
}
