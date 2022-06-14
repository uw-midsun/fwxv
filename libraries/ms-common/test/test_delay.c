#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "task.h"

void setup_test(void) {}

void teardown_test(void) {}

void test_delay_ms(void) {
  log_init();
  LOG_DEBUG("Begin Delay\n");
  delay_ms(500);
  LOG_DEBUG("End Delay\n");
}
