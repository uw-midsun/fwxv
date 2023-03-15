#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "semaphore.h"
#include "steering_analog_task.h"
#include "steering_tx_structs.h"
#include "task_test_helpers.h"
#include "unity.h"

#define CONTROL_STALK_STATE g_tx_struct.steering_info_analog_input

static const GpioAddress s_ctrl_stk_address = CTRL_STALK_GPIO;
uint16_t control_stalk_data;

void setup_test(void) {
  log_init();
}

void teardown_test(void) {}

TASK(steering_analog_input_left, TASK_STACK_512) {
  LOG_DEBUG("steering_analog_input_left started\n");
  steering_analog_input();
  while (true) {
  }
}

TASK(steering_analog_input_right, TASK_STACK_512) {
  LOG_DEBUG("steering_analog_input_right started\n");
  steering_analog_input();
  while (true) {
  }
}

TASK(steering_analog_input_off, TASK_STACK_512) {
  LOG_DEBUG("steering_analog_input_off started\n");
  steering_analog_input();
  while (true) {
  }
}

TEST_IN_TASK
void test_steering_analog_left(void) {
  adc_init(ADC_MODE_SINGLE);
  control_stalk_data = STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE_MV;
  set_reading(s_ctrl_stk_address, &control_stalk_data);
  delay_ms(20);
  adc_read_converted(s_ctrl_stk_address, &control_stalk_data);
  tasks_init_task(steering_analog_input_left, TASK_PRIORITY(1), NULL);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(g_tx_struct.steering_info_analog_input, STEERING_LIGHT_LEFT);
}

TEST_IN_TASK
void test_steering_analog_right(void) {
  adc_init(ADC_MODE_SINGLE);
  control_stalk_data = STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE_MV;
  set_reading(s_ctrl_stk_address, &control_stalk_data);
  tasks_init_task(steering_analog_input_right, TASK_PRIORITY(2), NULL);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(g_tx_struct.steering_info_analog_input, STEERING_LIGHT_RIGHT);
}

TEST_IN_TASK
void test_steering_analog_off(void) {
  adc_init(ADC_MODE_SINGLE);
  control_stalk_data = 0;
  set_reading(s_ctrl_stk_address, &control_stalk_data);
  tasks_init_task(steering_analog_input_off, TASK_PRIORITY(3), NULL);
  wait_tasks(1);
  TEST_ASSERT_EQUAL(g_tx_struct.steering_info_analog_input, STEERING_LIGHT_OFF);
}
