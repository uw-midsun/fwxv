#include <stdint.h>

#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "task.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}










static StackType_t s_task_stack;
static StaticTask_t s_task_tcb;
static StackType_t s_task_stack_2;
static StaticTask_t s_task_tcb_2;

static bool triggered = false;

// higher prioority task triggered by gpio
static void interrupt_handler() {
  uint32_t value;
  while (true) {
    xTaskNotifyWait(0, UINT32_MAX, &value, portMAX_DELAY);
    triggered = true;
  }
}

static GpioAddress addr = { .port = GPIO_PORT_A, .pin = 5 };
static GpioAddress addr2 = { .port = GPIO_PORT_A, .pin = 5 };
static InterruptSettings settings = {
  .priority = INTERRUPT_PRIORITY_NORMAL,
  .type = INTERRUPT_TYPE_INTERRUPT,
};

static void prv_test_trigger(void) {
  for (int i = 0; i < 5; ++i) {
    delay_ms(50);
    gpio_it_trigger_interrupt(&addr);
    TEST_ASSERT(triggered);

    triggered = false;
  }
  vTaskEndScheduler();
}

void test_gpio_it() {
  TaskHandle_t handler = xTaskCreateStatic(interrupt_handler, "RECEIVE", configMINIMAL_STACK_SIZE, NULL,
                              tskIDLE_PRIORITY + 2, &s_task_stack, &s_task_tcb);
  LOG_DEBUG("%p\n", handler);
  TaskHandle_t send_handle =
      xTaskCreateStatic(prv_test_trigger, "SEND", configMINIMAL_STACK_SIZE, NULL,
                        tskIDLE_PRIORITY + 1, &s_task_stack_2, &s_task_tcb_2);
  interrupt_init();
  gpio_it_init();
  gpio_it_register_interrupt(&addr, &settings, INTERRUPT_EDGE_RISING_FALLING, handler, 0);

  vTaskStartScheduler();
}

//
// TEST 2
//

void prv_test_2() {

}











void test_gpio_it_0() {
  TaskHandle_t handler = xTaskCreateStatic(interrupt_handler, "RECEIVE", configMINIMAL_STACK_SIZE, NULL,
                              tskIDLE_PRIORITY + 1, &s_task_stack, &s_task_tcb);
  LOG_DEBUG("%p\n", handler);
  TaskHandle_t send_handle =
      xTaskCreateStatic(prv_test_trigger, "SEND", configMINIMAL_STACK_SIZE, NULL,
                        tskIDLE_PRIORITY + 1, &s_task_stack_2, &s_task_tcb_2);
  interrupt_init();
  gpio_it_init();
  gpio_it_register_interrupt(&addr, &settings, INTERRUPT_EDGE_RISING_FALLING, handler, 0);

  vTaskStartScheduler();
}

