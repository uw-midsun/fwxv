#include "gpio_it.h"

#include <stdint.h>
#include <stdlib.h>

#include "gpio.h"
#include "interrupt_def.h"
#include "notify.h"
#include "status.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_interrupt.h"

typedef struct GpioInterrupt {
  InterruptSettings settings;
  GpioAddress address;
  Event event;
  Task *task;
} GpioInterrupt;

static GpioInterrupt s_gpio_it_interrupts[GPIO_PINS_PER_PORT];

void gpio_it_init(void) {
  GpioInterrupt empty_interrupt = { 0 };
  for (int16_t i = 0; i < GPIO_PINS_PER_PORT; i++) {
    s_gpio_it_interrupts[i] = empty_interrupt;
  }
}

StatusCode gpio_it_get_edge(const GpioAddress *address, InterruptEdge *edge) {
  if (s_gpio_it_interrupts[address->pin].task != NULL) {
    *edge = s_gpio_it_interrupts[address->pin].settings.edge;
    return STATUS_CODE_OK;
  }
  return STATUS_CODE_UNINITIALIZED;
}

StatusCode gpio_it_register_interrupt(const GpioAddress *address, const InterruptSettings *settings,
                                      const Event event, const Task *task) {
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    return STATUS_CODE_UNREACHABLE;
  } else if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
             event >= INVALID_EVENT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_gpio_it_interrupts[address->pin].task != NULL) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Pin already used.");
  }

  // Register exti channel and enable interrupt
  status_ok_or_return(stm32f10x_interrupt_exti_enable(address, settings));

  s_gpio_it_interrupts[address->pin].address = *address;
  s_gpio_it_interrupts[address->pin].settings = *settings;
  s_gpio_it_interrupts[address->pin].event = event;
  s_gpio_it_interrupts[address->pin].task = task;

  return STATUS_CODE_OK;
}

StatusCode gpio_it_trigger_interrupt(const GpioAddress *address) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  return stm32f10x_interrupt_exti_trigger(address->pin);
}

// Callback runner for GPIO which runs callbacks based on which callbacks are
// associated with an IRQ channel. The function runs the callbacks which have a
// flag raised in the range [lower_bound, upperbound].
static void prv_run_gpio_callbacks(uint8_t lower_bound, uint8_t upper_bound) {
  uint8_t pending = 0;
  for (int i = lower_bound; i <= upper_bound; i++) {
    stm32f10x_interrupt_exti_get_pending(i, &pending);
    if (pending && s_gpio_it_interrupts[i].task != NULL) {
      notify_from_isr(s_gpio_it_interrupts[i].task, s_gpio_it_interrupts[i].event);
    }
    stm32f10x_interrupt_exti_clear_pending(i);
  }
}

// IV Handler for pins 0.
void EXTI0_IRQHandler(void) {
  prv_run_gpio_callbacks(0, 0);
}

// IV Handler for pins 1.
void EXTI1_IRQHandler(void) {
  prv_run_gpio_callbacks(1, 1);
}

// IV Handler for pins 2.
void EXTI2_IRQHandler(void) {
  prv_run_gpio_callbacks(2, 2);
}

// IV Handler for pins 3.
void EXTI3_IRQHandler(void) {
  prv_run_gpio_callbacks(3, 3);
}

// IV Handler for pins 4.
void EXTI4_IRQHandler(void) {
  prv_run_gpio_callbacks(4, 4);
}

// IV Handler for pins 5 to 9.
void EXTI9_5_IRQHandler(void) {
  prv_run_gpio_callbacks(5, 9);
}

// IV Handler for pins 10 to 15.
void EXTI15_10_IRQHandler(void) {
  prv_run_gpio_callbacks(10, 15);
}

StatusCode gpio_it_mask_interrupt(const GpioAddress *address, bool masked) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return stm32f10x_interrupt_exti_mask_set(address->pin, masked);
}
