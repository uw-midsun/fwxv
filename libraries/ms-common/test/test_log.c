// testing file for log.c

#include "delay.h"
#include "log.h"
#include "task_test_helpers.h"
#include "tasks.h"

void setup_test(void) {
  log_init();
}

// uncoment when testing log_debug for no scheduler
// void test_log_no_scheduler(){
//   LOG_DEBUG("this is a log debug \n");
//   LOG_CRITICAL("This should be in between the log debugs \n");
//   LOG_WARN("im afraid youre schedulerless\n");
// }

TASK_TEST(test_log, TASK_STACK_512) {
  // test normal log
  // test log without scheduler (idk if this is possible)

  // test log when the array gets filled up
  // test all criticals
  // test all debugs
  /// test long string MAX string size 200
  // Log the different variable types

  delay_ms(1);

  for(int i = 1; i <= 33; i++){
    LOG_DEBUG("%i \n", i);
    // should not print 33
  }

  delay_ms(50);

  for(int i = 1; i <= 33; i++){
    LOG_CRITICAL("%i \n", i);
    // should not print 33
    // 1 should be at the back
  }

  delay_ms(50);

  // string log
  char trash[] = "Shichengs office hours";
  LOG_DEBUG("This is a string log: %s \n", trash );

  // int log
  LOG_DEBUG("This is a positive int log: %i \n", 69 );
  LOG_DEBUG("This is a negative int log: %i \n", -420 );

  // double log
  LOG_DEBUG("This is a positive double log: %f \n", 69.420 );
  LOG_DEBUG("This is a negative double log: %f \n", -420.69 );
  LOG_DEBUG("This is a capped (at 3 decimals) double log: %.3f \n", -3.454234);

  // boolean log
  LOG_DEBUG("The statement below me is true: %i \n", true ); // actually true
  LOG_DEBUG("The statement above me is false: %i \n", false ); // actually false

  // long int log
  LOG_DEBUG("This is a positive long int log: %li \n", 69311489273489724 );
  LOG_DEBUG("This is a negative long int log: %li \n", -69311489273489724 );

  // pointer log
  LOG_DEBUG("This is a pointer: %p \n", trash );

  // testing multiple arguments
  LOG_DEBUG("This is everything: %s, %i, %.3f, %li, %p \n", trash, 69, 69.420, 69311489273489724, trash );

  delay_ms(50);

}
