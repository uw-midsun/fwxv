// testing file for log.c in a taskt

#include "delay.h"
#include "log.h"
#include "task_test_helpers.h"
#include "tasks.h"

void setup_test(void) {
  log_init();
}

TASK(blink_task, TASK_STACK_512) {
  // should print
  // "Task blink_task starting."
  // "blink first"
  // "blink critical"
  // "blink second"
  // for two rounds
  while (true) {
    LOG_DEBUG("blink first\n");
    delay_ms(2);
    LOG_DEBUG("blink second\n");
    LOG_CRITICAL("blink critical\n");
    delay_ms(2);
  }
}

TASK_TEST(test_log, TASK_STACK_512) {
  delay_ms(1);

  for (int i = 1; i <= 33; i++) {
    LOG_DEBUG("%i \n", i);
    // should not print 33
    // should give "WARNING: Log queue is full"
  }

  delay_ms(50);

  for (int i = 1; i <= 33; i++) {
    LOG_CRITICAL("%i \n", i);
    // should not print 33
    // 1 should be at the back
    // should give "WARNING: Log queue is full"
  }

  delay_ms(50);

  // string log
  char trash[] = "Shichengs office hours";
  LOG_DEBUG("This is a string log: %s \n", trash);

  // int log
  LOG_DEBUG("This is a positive int log: %i \n", 69);
  LOG_DEBUG("This is a negative int log: %i \n", -420);

  // double log
  LOG_DEBUG("This is a positive double log: %f \n", 69.420);
  LOG_DEBUG("This is a negative double log: %f \n", -420.69);
  LOG_DEBUG("This is a capped (at 3 decimals) double log: %.3f \n", -3.454234);

  // boolean log
  LOG_DEBUG("The statement below me is true: %i \n", true);    // actually true
  LOG_DEBUG("The statement above me is false: %i \n", false);  // actually false

  // long int log
  LOG_DEBUG("This is a positive long int log: %li \n", 69311489273489724);
  LOG_DEBUG("This is a negative long int log: %li \n", -69311489273489724);

  // pointer log
  LOG_DEBUG("This is a pointer: %p \n", trash);

  // testing multiple arguments
  LOG_DEBUG("This is everything: %s, %i, %.3f, %li, %p \n", trash, 69, 69.420, 69311489273489724,
            trash);

  delay_ms(50);

  tasks_init_task(blink_task, TASK_PRIORITY(2), NULL);

  delay_ms(8);
}
