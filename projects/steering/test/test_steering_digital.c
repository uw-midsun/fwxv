#include "can.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "notify.h"
#include "status.h"
#include "steering_digital_task.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define DEVICE_ID 0x04
#define INVALID_STEERING_CAN_EVENT \
  { .id = 16, .data = 0 }

#define DIGITAL_INPUT g_tx_struct.steering_info_digital_input

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static uint32_t notification = 0;
static Event steering_event;

TASK(handler, TASK_MIN_STACK_SIZE) {
  while (true) {
    steering_digital_input();
  }
}

// Setup test
void setup_test(void) {
  tasks_init_task(handler, 2, NULL);
  gpio_it_init();
  TEST_ASSERT_OK(steering_digital_input_init(handler));
}

void teardown_test(void) {}

TEST_IN_TASK
void test_steering_input(void) {
  // Empty notification
  TEST_ASSERT_EQUAL(notification, 0);
  TEST_ASSERT_EQUAL(steering_event, INVALID_EVENT);
}

TEST_IN_TASK
void test_steering_digital_input_horn() {
  // Test horn event & CAN message - press and unpress
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)HORN_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_HORN_MASK, DIGITAL_SIGNAL_HORN_MASK);

  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)HORN_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_HORN_MASK, 0);
}

TEST_IN_TASK
void test_steering_cc_toggle() {
  // Test CC toggle event & CAN message - press and unpress
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)CC_TOGGLE_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_CC_TOGGLE_MASK, DIGITAL_SIGNAL_CC_TOGGLE_MASK);

  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)CC_TOGGLE_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_CC_TOGGLE_MASK, 0);
}

TEST_IN_TASK
void test_steering_regen_brake() {
  // Test Regen brake event & CAN message - press and unpress
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)REGEN_BRAKE_TOGGLE_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_REGEN_BRAKE_MASK,
                    DIGITAL_SIGNAL_REGEN_BRAKE_MASK);

  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)REGEN_BRAKE_TOGGLE_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_REGEN_BRAKE_MASK, 0);
}

TEST_IN_TASK
void test_steering_cc_increase_decrease() {
  // Test CC increase speed event & CAN message - press
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)CC_INCREASE_SPEED_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_CC_INCREASE_MASK,
                    DIGITAL_SIGNAL_CC_INCREASE_MASK);

  // Test CC decrease speed event & CAN message - press
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&(GpioAddress)CC_DECREASE_SPEED_GPIO_ADDR));
  wait_tasks(1);
  TEST_ASSERT_EQUAL(DIGITAL_INPUT & DIGITAL_SIGNAL_CC_DECREASE_MASK,
                    DIGITAL_SIGNAL_CC_DECREASE_MASK);
}
