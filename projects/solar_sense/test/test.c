#include <stdio.h>

#include "log.h"
#include "master_task.h"
#include "solar_sense_getters.h"
#include "task_test_helpers.h"
#include "tasks.h"
#include "unity.h"

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

TEST_IN_TASK
void test_setting_can_msg_by_pointer_offset(void) {
  for (int i = 0; i < 6; ++i) {
    *(&g_tx_struct.mppt_1_current + 4 * i) = i + 1;
    *(&g_tx_struct.mppt_1_voltage + 4 * i) = i + 11;
    *(&g_tx_struct.mppt_1_pwm + 4 * i) = i + 21;
    *(&g_tx_struct.mppt_1_status + 4 * i) = i + 31;
  }

  TEST_ASSERT_EQUAL(1, g_tx_struct.mppt_1_current);
  TEST_ASSERT_EQUAL(2, g_tx_struct.mppt_2_current);
  TEST_ASSERT_EQUAL(3, g_tx_struct.mppt_3_current);
  TEST_ASSERT_EQUAL(4, g_tx_struct.mppt_4_current);
  TEST_ASSERT_EQUAL(5, g_tx_struct.mppt_5_current);
  TEST_ASSERT_EQUAL(6, g_tx_struct.mppt_6_current);

  TEST_ASSERT_EQUAL(11, g_tx_struct.mppt_1_voltage);
  TEST_ASSERT_EQUAL(12, g_tx_struct.mppt_2_voltage);
  TEST_ASSERT_EQUAL(13, g_tx_struct.mppt_3_voltage);
  TEST_ASSERT_EQUAL(14, g_tx_struct.mppt_4_voltage);
  TEST_ASSERT_EQUAL(15, g_tx_struct.mppt_5_voltage);
  TEST_ASSERT_EQUAL(16, g_tx_struct.mppt_6_voltage);

  TEST_ASSERT_EQUAL(21, g_tx_struct.mppt_1_pwm);
  TEST_ASSERT_EQUAL(22, g_tx_struct.mppt_2_pwm);
  TEST_ASSERT_EQUAL(23, g_tx_struct.mppt_3_pwm);
  TEST_ASSERT_EQUAL(24, g_tx_struct.mppt_4_pwm);
  TEST_ASSERT_EQUAL(25, g_tx_struct.mppt_5_pwm);
  TEST_ASSERT_EQUAL(26, g_tx_struct.mppt_6_pwm);

  TEST_ASSERT_EQUAL(31, g_tx_struct.mppt_1_status);
  TEST_ASSERT_EQUAL(32, g_tx_struct.mppt_2_status);
  TEST_ASSERT_EQUAL(33, g_tx_struct.mppt_3_status);
  TEST_ASSERT_EQUAL(34, g_tx_struct.mppt_4_status);
  TEST_ASSERT_EQUAL(35, g_tx_struct.mppt_5_status);
  TEST_ASSERT_EQUAL(36, g_tx_struct.mppt_6_status);
}
