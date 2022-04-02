// testing file for log.c when scheduler is off

#include "delay.h"
#include "log.h"
#include "task_test_helpers.h"
#include "tasks.h"

void setup_test(void) {
  log_init();
}

void test_log_no_scheduler(){
  LOG_DEBUG("this is a log debug \n");
  LOG_CRITICAL("This should be in between the log debugs \n");
  LOG_WARN("im afraid youre schedulerless\n");
}
