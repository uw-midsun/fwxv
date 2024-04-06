#include "can_datagram.h"

#include "can.h"
#include "log.h"
#include "unity.h"

#define TEST_CAN_DEVICE_ID 1
#define TEST_CAN_START_MSG_ID 3
#define TEST_CAN_DGRAM_MSG_ID_TX 1  // MSG ID USED TO TEST DGRAM TX
#define TEST_CAN_DGRAM_MSG_ID_RX 2  // MSG ID USED TO TEST DGRAM RX
#define TEST_CAN_BUFFER_SIZE 8

// Brought in frmo can_datagram.h
#define DGRAM_MAX_DATA_SIZE 2048

#define DGRAM_TYPE_SIZE_BYTES 1
#define CRC_SIZE_BYTES 4
#define NODE_IDS_SIZE_BYTES 2
#define DATA_LEN_SIZE_BYTES 2

#define TEST_DATA_SIZE_SHORT 16
#define NUM_SHORT_TEST_MSG 6

#define TEST_DATA_SIZE_LONG 2048
#define NUM_LONG_TEST_MSG 289

static CanStorage s_can_storage;

static uint8_t s_data[TEST_DATA_SIZE_SHORT] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                                'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };

static uint8_t s_test_short_nodes[3] = { 1, 4, 3 };

static uint8_t data_lengths[] = { 1, 4, 1, 8, 2, 8};
static int s_num_msg_rx;
static int s_num_tx;
static bool s_start_message_set;

void setup_test(void) {
//   interrupt_init();
//   soft_timer_init();
//   crc32_init();
}

void teardown_test(void) {\
  s_start_message_set = false;
  s_num_msg_rx = 0;
  s_num_tx = 0;
  s_num_msg_rx = 0;
}

static void prv_can_init() {
  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message) {
  CanMessage msg;
  LOG_DEBUG("CALLBACK\n");
  return STATUS_CODE_OK;
}


void test_can_datagram_tx(void) {
    prv_can_init();
    can_datagram_init();

    CanDatagramTxConfig tx_config = {
        .dgram_type = 3,
        .node_ids_list = s_test_short_nodes,
        .node_ids_length = 3,
        .data_len = TEST_DATA_SIZE_SHORT,
        .data = s_data,
        .tx_cb = prv_tx_callback,
    };

    can_datagram_start_tx(&tx_config);
    TEST_ASSERT_EQUAL(DATAGRAM_STATUS_ACTIVE, can_datagram_get_status());

    // while (can_datagram_get_status() == DATAGRAM_STATUS_ACTIVE) {
    //     //process event
    //     break;
    // }

    // TEST_ASSERT_EQUAL(DATAGRAM_STATUS_TX_COMPLETE, can_datagram_get_status());

}
