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
  uint32_t value;
  while (true) {
    xTaskNotifyWait(0, UINT32_MAX, &value, portMAX_DELAY);

    // value holds the bitset of notified values.
    if (value & 0b1) {
      LOG_DEBUG("notified at 0");
    } else {
      LOG_DEBUG("%x", value);
    }
  }
}

void main() {
  TaskHandle_t handle = xTaskCreateStatic(prv_higher_priority, "RECEIVE", configMINIMAL_STACK_SIZE,
                                          NULL, tskIDLE_PRIORITY + 3, &s_task_stack, &s_task_tcb);
  GpioAddress addr = { .port = GPIO_PORT_A, .pin = 5 };
  InterruptSettings settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
  };
  gpio_it_register_interrupt(&addr, &settings, INTERRUPT_EDGE_RISING_FALLING, handle, 0);
}