#include <stdint.h>

#include "gpio_it.h"
#include "log.h"
#include "task.h"

void setup_test(void) {}

void teardown_test(void) {}

static StackType_t s_task_stack;
static StaticTask_t s_task_tcb;

static bool triggered = false;

// higher prioority task triggered by gpio
static void prv_higher_priority(void) {
  while (true) {

    
    triggered = true;
  }
}

void main() {
  xTaskCreateStatic(prv_higher_priority, "RECEIVE", configMINIMAL_STACK_SIZE, NULL,
                    tskIDLE_PRIORITY + 3, &s_task_stack, &s_task_tcb);
  
}