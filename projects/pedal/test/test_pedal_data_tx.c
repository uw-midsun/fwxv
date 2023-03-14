#include <stdint.h>
#include "max11600.h"
#include "can_hw.h"
#include "can_msg.h"
#include "can.h"
#include "delay.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "pedal_data.h"
#include "soft_timer.h"
#include "unity.h"

// Define constants for CAN message IDs, types, and DLC
#define PEDAL_CAN_MSG_ID 18
#define PEDAL_CAN_MSG_TYPE CAN_MSG_TYPE_DATA
#define PEDAL_CAN_MSG_EXTENDED true
#define PEDAL_CAN_MSG_DLC 8

// Define constants for throttle and brake values
#define THROTTLE_POSITION 1
#define BRAKE_POSITION 1


static CanStorage can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

// Function to send a CAN message containing the throttle and brake readings
StatusCode transmit_pedal_data(int16_t throttle_reading, int16_t brake_reading) {
  // Create a new CAN message with the throttle and brake readings
  CanMessage msg = {
    .id = { .raw = PEDAL_CAN_MSG_ID },
    .type = PEDAL_CAN_MSG_TYPE,
    .extended = PEDAL_CAN_MSG_EXTENDED,
    .dlc = PEDAL_CAN_MSG_DLC,
    .data_u16 = { (uint16_t)throttle_reading, (uint16_t)brake_reading }
  };

  // Send the CAN message
  return can_transmit(&msg);
}

// Define a static variable to hold the sent CAN message
static CanMessage sent_msg;

// Define static variables to hold the simulated throttle and brake readings
static int16_t simulated_throttle_reading = THROTTLE_POSITION;
static int16_t simulated_brake_reading = BRAKE_POSITION;

// Mock CAN transmit function to capture the transmitted message
StatusCode TEST_MOCK(can_transmit)(const CanMessage *msg) {
  sent_msg = *msg;
  return STATUS_CODE_OK;
}

// Mock function for reading throttle data from MAX11600
StatusCode TEST_MOCK(read_throttle_data)(int16_t *position) {
  *position = simulated_throttle_reading;
  return STATUS_CODE_OK;
}

// Mock function for reading brake data from MAX11600
StatusCode TEST_MOCK(read_brake_data)(int16_t *position) {
  *position = simulated_brake_reading;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  log_init();
  can_init(&can_storage, &can_settings);
}

void teardown_test(void) {}

void test_pedal_can_tx(void) {
  // Send the throttle and brake values over CAN
  TEST_ASSERT_OK(transmit_pedal_data(simulated_throttle_reading, simulated_brake_reading));

  // Check the sent CAN message ID, type, and DLC
  TEST_ASSERT_EQUAL_HEX32(PEDAL_CAN_MSG_ID, sent_msg.id.raw);
  TEST_ASSERT_EQUAL_UINT8(PEDAL_CAN_MSG_TYPE, sent_msg.type);
  TEST_ASSERT_EQUAL_UINT8(PEDAL_CAN_MSG_EXTENDED, sent_msg.extended);
  TEST_ASSERT_EQUAL_UINT8(PEDAL_CAN_MSG_DLC, sent_msg.dlc);

  // Check the throttle and brake signals in the CAN message
  int32_t throttle_sent = (int32_t)(sent_msg.data_u16[0]);
  int32_t brake_sent = (int32_t)(sent_msg.data_u16[1]);
  TEST_ASSERT_EQUAL_INT32(simulated_throttle_reading, throttle_sent);
  TEST_ASSERT_EQUAL_INT32(simulated_brake_reading, brake_sent);
}
