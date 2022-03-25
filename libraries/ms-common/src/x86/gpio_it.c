#include "gpio_it.h"

#include <stdint.h>
#include <stdlib.h>

#include "gpio.h"
#include "interrupt_def.h"
#include "status.h"
#include "x86_interrupt.h"

typedef struct GpioItInterrupt {
  uint8_t interrupt_id;
  InterruptEdge edge;
  GpioAddress address;
  TaskHandle_t task;
  uint8_t bit;
} GpioItInterrupt;

static uint8_t s_gpio_it_handler_id;
static GpioItInterrupt s_gpio_it_interrupts[GPIO_PINS_PER_PORT];

static void prv_gpio_it_handler(uint8_t interrupt_id) {
  for (int i = 0; i < GPIO_PINS_PER_PORT; ++i) {
    if (s_gpio_it_interrupts[i].task != NULL &&
        s_gpio_it_interrupts[i].interrupt_id == interrupt_id) {
      BaseType_t higher_priority_task_woken = pdFALSE;
      xTaskNotifyFromISR(s_gpio_it_interrupts[i].task, 1u << s_gpio_it_interrupts[i].bit, eSetBits,
                         &higher_priority_task_woken);
      portYIELD_FROM_ISR(higher_priority_task_woken);
    }
  }
}

void gpio_it_init(void) {
  x86_interrupt_register_handler(prv_gpio_it_handler, &s_gpio_it_handler_id);
  GpioItInterrupt empty_interrupt = { 0 };
  for (uint16_t i = 0; i < GPIO_PINS_PER_PORT; i++) {
    s_gpio_it_interrupts[i] = empty_interrupt;
  }
}

StatusCode gpio_it_get_edge(const GpioAddress *address, InterruptEdge *edge) {
  if (s_gpio_it_interrupts[address->pin].task != NULL) {
    *edge = s_gpio_it_interrupts[address->pin].edge;
    return STATUS_CODE_OK;
  }
  return STATUS_CODE_UNINITIALIZED;
}

StatusCode gpio_it_register_interrupt(const GpioAddress *address, const InterruptSettings *settings,
                                      InterruptEdge edge, TaskHandle_t task_to_notify,
                                      uint8_t bit_to_set) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT || bit_to_set > 5) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_gpio_it_interrupts[address->pin].task != NULL) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Pin already in use.");
  }

  uint8_t interrupt_id;
  status_ok_or_return(
      x86_interrupt_register_interrupt(s_gpio_it_handler_id, settings, &interrupt_id));

  s_gpio_it_interrupts[address->pin].interrupt_id = interrupt_id;
  s_gpio_it_interrupts[address->pin].edge = edge;
  s_gpio_it_interrupts[address->pin].address = *address;
  s_gpio_it_interrupts[address->pin].task = task_to_notify;
  s_gpio_it_interrupts[address->pin].bit = bit_to_set;

  return STATUS_CODE_OK;
}

StatusCode gpio_it_trigger_interrupt(const GpioAddress *address) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return x86_interrupt_trigger(s_gpio_it_interrupts[address->pin].interrupt_id);
}

StatusCode gpio_it_mask_interrupt(const GpioAddress *address, bool masked) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
