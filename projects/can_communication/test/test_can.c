#include "can.h"
#include "can_board_ids.h"
#include "can_communication_getters.h"
#include "can_communication_setters.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "task_test_helpers.h"
#include "unity.h"

#define DEAD 0xDEAD
#define BEEF 0xBEEF

static CanStorage s_can_storage = { 0 };
const CanSettings s_can_settings = {
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static uint8_t s_to_transmit;

void setup_test(void) {}

void set_outputs() {
  if (s_to_transmit) {
    set_one_shot_msg_sig1(DEAD);
    set_one_shot_msg_sig2(BEEF);
  } else {
    set_one_shot_msg_sig1(BEEF);
    set_one_shot_msg_sig2(DEAD);
  }
}

void check_inputs() {
  if (s_to_transmit) {
    TEST_ASSERT_EQUAL(DEAD, get_one_shot_msg_sig1());
    TEST_ASSERT_EQUAL(BEEF, get_one_shot_msg_sig2());
  } else {
    TEST_ASSERT_EQUAL(BEEF, get_one_shot_msg_sig1());
    TEST_ASSERT_EQUAL(DEAD, get_one_shot_msg_sig2());
  }
}

void teardown_test(void) {}

TEST_IN_TASK
void test_can_task(void) {
  log_init();
  gpio_init();
  can_init(&s_can_storage, &s_can_settings);

  s_to_transmit = 0;

  LOG_DEBUG("Testing CAN architecture!\n");
  // Init Cycle
  run_can_rx_cycle();
  wait_tasks(1);
  TEST_ASSERT_EQUAL(get_one_shot_msg_sig1(), 0);
  TEST_ASSERT_EQUAL(get_one_shot_msg_sig2(), 0);

  s_to_transmit = (s_to_transmit + 1) % 2;
  set_outputs();

  run_can_tx_cycle();
  wait_tasks(1);

  // Cycle 5 times
  for (size_t i = 0; i < 5; ++i) {
    run_can_rx_cycle();
    wait_tasks(1);
    check_inputs();

    s_to_transmit = (s_to_transmit + 1) % 2;
    set_outputs();

    run_can_tx_cycle();
    wait_tasks(1);
  }
}
