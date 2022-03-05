<<<<<<< HEAD
=======
#include "FreeRTOS.h"
#include "log.h"
#include "task.h"

>>>>>>> main
#include <stdint.h>

#include "FreeRTOS.h"
#include "delay.h"
#include "log.h"
#include "task.h"

void setup_test(void) {}

void teardown_test(void) {}

static StackType_t s_task_stack;
static StaticTask_t s_task_tcb;

static void delay_task() {
  LOG_DEBUG("Begin Delay\n");
  delay_ms(500);
  LOG_DEBUG("End Delay\n");
  vTaskEndScheduler();
}

void test_delay_ms(void) {
  xTaskCreateStatic(delay_task, "Delay", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,
                    &s_task_stack, &s_task_tcb);

  vTaskStartScheduler();
}
