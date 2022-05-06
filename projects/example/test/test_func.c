#include "func.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}

void test_func(void) {
    int ret = get5();
    TEST_ASSERT_EQUAL(5, ret);
}
