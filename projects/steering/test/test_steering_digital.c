#include "can.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "notify.h"
#include "status.h"
#include "steering_digital_task.h"
#include "task_test_helpers.h"
#include "test_helpers.h"
#include "tests.h"
#include "unity.h"

#define DEVICE_ID 0x04
#define INVALID_STEERING_CAN_EVENT \
  { .id = 16, .data = 0 }

#define DIGITAL_INPUT g_tx_struct.steering_info_digital_input

GpioAddress HORN = HORN_GPIO_ADDR;
GpioAddress CC_TOGGLE = CC_TOGGLE_GPIO_ADDR;
GpioAddress REGEN_BRAKE = REGEN_BRAKE_TOGGLE_GPIO_ADDR;
GpioAddress CC_DECREASE = CC_DECREASE_SPEED_GPIO_ADDR;
GpioAddress CC_INCREASE = CC_INCREASE_SPEED_GPIO_ADDR;

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static Semaphore sem_a;
static Semaphore sem_b;

TASK(digital_horn_input, TASK_STACK_512) {
  LOG_DEBUG("digital_horn_input task started\n");
  while (true) {
    // sem_wait(&sem, BLOCK_INDEFINITELY);
    test_start_take();
    steering_digital_input();
  }
}

TASK(digital_cc_toggle_input, TASK_STACK_512) {
  LOG_DEBUG("digital_cc_toggle_input task started\n");
  while (true) {
    test_start_take();
    steering_digital_input();
  }
}

TASK(digital_regen_brake_input, TASK_STACK_512) {
  LOG_DEBUG("digital_regen_brake_input task started\n");
  while (true) {
    test_start_take();
    steering_digital_input();
  }
}

TASK(digital_cc_increase_decrease_input, TASK_STACK_512) {
  LOG_DEBUG("digital_cc_increase_decrease_input task started\n");
  while (true) {
    steering_digital_input();
    sem_post(&sem_b);
    sem_wait(&sem_a, BLOCK_INDEFINITELY);
  }
}

// Setup test
void setup_test(void) {
  gpio_it_init();
  sem_init(&sem_a, 1, 0);
  sem_init(&sem_b, 1, 0);
  tests_init();
}

void teardown_test(void) {}

TEST_IN_TASK
void test_steering_digital_input_horn() {
  steering_digital_input_init(digital_horn_input);
  tasks_init_task(digital_horn_input, TASK_PRIORITY(1), NULL);
  // Test horn event & CAN message - press and unpress
  notify(digital_horn_input, STEERING_INPUT_HORN_EVENT);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(DIGITAL_SIGNAL_HORN_MASK, DIGITAL_INPUT & DIGITAL_SIGNAL_HORN_MASK);

  notify(digital_horn_input, STEERING_INPUT_HORN_EVENT);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(0, DIGITAL_INPUT & DIGITAL_SIGNAL_HORN_MASK);
}

TEST_IN_TASK
void test_steering_cc_toggle() {
  steering_digital_input_init(digital_cc_toggle_input);
  tasks_init_task(digital_cc_toggle_input, TASK_PRIORITY(2), NULL);
  // Test CC toggle event & CAN message - press and unpress
  notify(digital_cc_toggle_input, STEERING_CC_TOGGLE_EVENT);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(DIGITAL_SIGNAL_CC_TOGGLE_MASK, DIGITAL_INPUT & DIGITAL_SIGNAL_CC_TOGGLE_MASK);

  notify(digital_cc_toggle_input, STEERING_CC_TOGGLE_EVENT);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(0, DIGITAL_INPUT & DIGITAL_SIGNAL_CC_TOGGLE_MASK);
}

TEST_IN_TASK
void test_steering_regen_brake() {
  steering_digital_input_init(digital_regen_brake_input);
  tasks_init_task(digital_regen_brake_input, TASK_PRIORITY(3), NULL);
  // Test Regen brake event & CAN message - press and unpress
  notify(digital_regen_brake_input, STEERING_REGEN_BRAKE_EVENT);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(DIGITAL_SIGNAL_REGEN_BRAKE_MASK,
                    DIGITAL_INPUT & DIGITAL_SIGNAL_REGEN_BRAKE_MASK);

  notify(digital_regen_brake_input, STEERING_REGEN_BRAKE_EVENT);
  test_start_give();
  test_end_take();
  TEST_ASSERT_EQUAL(0, DIGITAL_INPUT & DIGITAL_SIGNAL_REGEN_BRAKE_MASK);
}

TEST_IN_TASK
void test_steering_cc_increase_decrease() {
  steering_digital_input_init(digital_cc_increase_decrease_input);
  tasks_init_task(digital_cc_increase_decrease_input, TASK_PRIORITY(2), NULL);
  // Test CC increase speed event & CAN message - press

  notify(digital_cc_increase_decrease_input, STEERING_CC_INCREASE_SPEED_EVENT);
  sem_post(&sem_a);
  sem_wait(&sem_b, BLOCK_INDEFINITELY);
  TEST_ASSERT_EQUAL(DIGITAL_SIGNAL_CC_INCREASE_MASK,
                    DIGITAL_INPUT & DIGITAL_SIGNAL_CC_INCREASE_MASK);

  notify(digital_cc_increase_decrease_input, STEERING_CC_DECREASE_SPEED_EVENT);
  sem_post(&sem_a);
  sem_wait(&sem_b, BLOCK_INDEFINITELY);
  TEST_ASSERT_EQUAL(DIGITAL_SIGNAL_CC_DECREASE_MASK,
                    DIGITAL_INPUT & DIGITAL_SIGNAL_CC_DECREASE_MASK);
}
