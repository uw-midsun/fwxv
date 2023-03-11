#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "steering_analog_task.h"
#include "steering_tx_structs.h"
#include "task_test_helpers.h"
#include "unity.h"

#define CONTROL_STALK_STATE g_tx_struct.steering_info_analog_input

static bool s_task_started;
// Gpio Address can be changed to different values for testing
static const GpioAddress s_ctrl_stk_address = CTRL_STALK_GPIO;
uint16_t control_stalk_data = UINT16_MAX;
uint16_t control_stalk_data_actual = UINT16_MAX;

void setup_test(void) {
  s_task_started = false;
  log_init();
}

void teardown_test(void) {}

TASK(steering_analog_input_task, TASK_STACK_512) {
  LOG_DEBUG("steering_analog_input started\n");
  s_task_started = true;
  set_reading(s_ctrl_stk_address);
  control_stalk_data_actual = get_reading(s_ctrl_stk_address);
  steering_analog_input();
  while (true) {
  }
}

TEST_IN_TASK
void test_steering_analog(void) {
  adc_init(ADC_MODE_SINGLE);
  control_stalk_data = STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV;
  tasks_init_task(steering_analog_input_task, TASK_PRIORITY(1), NULL);
  TEST_ASSERT_FALSE(s_task_started);

  delay_ms(20);

  TEST_ASSERT_TRUE(s_task_started);
  TEST_ASSERT_EQUAL(g_tx_struct.steering_info_analog_input, STEERING_LIGHT_LEFT);
  s_task_started = false;

  control_stalk_data = STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV;
  tasks_init_task(steering_analog_input_task, TASK_PRIORITY(1), NULL);
  TEST_ASSERT_FALSE(s_task_started);

  delay_ms(20);

  TEST_ASSERT_TRUE(s_task_started);
  TEST_ASSERT_EQUAL(g_tx_struct.steering_info_analog_input, STEERING_LIGHT_RIGHT);
  s_task_started = false;

  control_stalk_data = 0;
  tasks_init_task(steering_analog_input_task, TASK_PRIORITY(1), NULL);
  TEST_ASSERT_FALSE(s_task_started);

  delay_ms(20);

  TEST_ASSERT_TRUE(s_task_started);
  TEST_ASSERT_EQUAL(g_tx_struct.steering_info_analog_input, STEERING_LIGHT_OFF);
}
