#include "pedal.h"
#include "task_test_helpers.h"
#include "unity.h"
#include "test_helpers.h"
#include "log.h"

static PedalCalibBlob mock_blob = { .throttle_calib = { .upper_value = 3300, .lower_value = 0 }};

void setup_test(void) {
pedal_init(&mock_blob);
}

void teardown_test(void){}

// INSERT FIRST TEST HERE
TEST_IN_TASK
void test_pedal_map(void) { 
    uint16_t mock1 = 0;
    uint16_t mock2 = 3300;
    LOG_DEBUG("UNIT TEST 1: ");
    pedal_map(mock1);
    LOG_DEBUG("UNIT TEST 2: ");
    pedal_map(mock2);
}