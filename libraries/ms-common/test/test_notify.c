#include <stdint.h> 
#include "FreeRTOS.h"  
#include "delay.h" 
#include "log.h" 
#include "task.h"
#include "notify.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_notify(void) {
  LOG_DEBUG("hellooo\n");
}
