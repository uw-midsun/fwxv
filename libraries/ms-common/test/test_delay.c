#include "delay.h"

#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}

void test_delay_us(void) {
  LOG_DEBUG("This is a message\n");
  delay_us(1000);
}
