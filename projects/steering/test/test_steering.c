#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "semaphore.h"
#include "steering_task.h"
#include "steering_tx_structs.h"
#include "task_test_helpers.h"
#include "tests.h"
#include "unity.h"

#define CC_INPUT g_tx_struct.steering_info_input_cc
#define STEERING_INPUT g_tx_struct.steering_info_input_lights

static const GpioAddress turn_signal_address = TURN_SIGNAL_GPIO;
static const GpioAddress cc_address = CC_CHANGE_GPIO;
static const GpioAddress cc_toggle_address = CC_TOGGLE_GPIO;

uint16_t control_stalk_data;

void setup_test(void) {
  log_init();
  adc_deinit();
  gpio_it_init();
  tests_init();
  steering_init();
  adc_init();
}

void teardown_test(void) {}

TASK(test_steering, TASK_STACK_512) {
  LOG_DEBUG("digital_cc_toggle_input task started\n");
  while (true) {
    test_start_take();
    steering_input();
    test_end_give();
  }
}

// Digital tasks
TEST_IN_TASK
void test_steering_cc_toggle() {
  tasks_init_task(test_steering, TASK_PRIORITY(2), NULL);
  // Test CC toggle event & CAN message - press
  notify(test_steering, CC_TOGGLE_EVENT);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(CC_TOGGLE_MASK, CC_INPUT & CC_TOGGLE_MASK);
}

// Analog tasks

TEST_IN_TASK
void test_cc_increase(void) {
  control_stalk_data = CRUISE_CONTROl_STALK_SPEED_INCREASE_VOLTAGE_MV;
  adc_set_reading(cc_address, control_stalk_data);
  tasks_init_task(test_steering, TASK_PRIORITY(4), NULL);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(CC_INCREASE_MASK, CC_INPUT & CC_INCREASE_MASK);
}

TEST_IN_TASK
void test_cc_decrease(void) {
  control_stalk_data = CRUISE_CONTROl_STALK_SPEED_DECREASE_VOLTAGE_MV;
  adc_set_reading(cc_address, control_stalk_data);
  tasks_init_task(test_steering, TASK_PRIORITY(5), NULL);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(CC_DECREASE_MASK, CC_INPUT & CC_DECREASE_MASK);
}

TEST_IN_TASK
void test_steering_analog_left(void) {
  control_stalk_data = TURN_LEFT_SIGNAL_VOLTAGE_MV;
  adc_set_reading(turn_signal_address, control_stalk_data);
  tasks_init_task(test_steering, TASK_PRIORITY(1), NULL);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(TURN_SIGNAL_LEFT, STEERING_INPUT);
}

TEST_IN_TASK
void test_steering_analog_right(void) {
  control_stalk_data = TURN_RIGHT_SIGNAL_VOLTAGE_MV;
  adc_set_reading(turn_signal_address, control_stalk_data);
  tasks_init_task(test_steering, TASK_PRIORITY(2), NULL);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(TURN_SIGNAL_RIGHT, STEERING_INPUT);
}

TEST_IN_TASK
void test_steering_analog_off(void) {
  control_stalk_data = NEUTRAL_SIGNAL_VOLTAGE_MV;
  adc_set_reading(turn_signal_address, control_stalk_data);
  tasks_init_task(test_steering, TASK_PRIORITY(3), NULL);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(TURN_SIGNAL_OFF, STEERING_INPUT);
}
