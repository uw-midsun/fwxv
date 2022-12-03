#include "stm32f10x_interrupt.h"

#include "status.h"
#include <stdbool.h>
#include <stdint.h>

#include "interrupt_def.h"
#include "status.h"
#include "misc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_rcc.h"

// Converts a line number into a EXTI_Linex value
#define EXTI_LINE(index) ((uint32_t)0x01<<(index))

// Converts our internal enum into an EXTIMode_TypeDef
#define EXTI_MODE(interrupt_type) (0x04 * interrupt_type)

// Converts our internal enum into an EXTITrigger_TypeDef
#define EXTI_TRIGGER(interrupt_edge) (0x08 + 0x04 * interrupt_edge)

// The exti peripheral maps each pin number [0-16] to a corresponding exti channel
// These channels are shared betwen ports (PA0, PB0, ..., PG0 map to EXTI_Channel0)
// STM32F10x provides 16 EXTI channels per gpio port (by which GPIO interrupts 
// can be processed), but only 7 possible IRQ channels
static uint8_t prv_get_irq_channel(uint8_t exti_channel) {
  switch(exti_channel) {
  case 0:
    return EXTI0_IRQn;
  case 1:
    return EXTI1_IRQn;
  case 2:
    return EXTI2_IRQn;
  case 3:
    return EXTI3_IRQn;
  case 4:
    return EXTI4_IRQn;
  default:
    if (exti_channel <= 9 && exti_channel >= 5) {
      return EXTI9_5_IRQn;
    } else {
      return EXTI15_10_IRQn;
    }
  }
}


void stm32f10x_interrupt_init(void) {
  // Enable clock interface for AFIO needed for exti
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  // Set Priority group register to use only preempt priority bits, no subpriority
  // This means we can have up to 16 priorities handled as preemption
  // https://electronics.stackexchange.com/q/384306
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

StatusCode stm32f10x_interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  if (priority >= NUM_INTERRUPT_PRIORITIES || irq_channel >= NUM_STM32F10X_INTERRUPT_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Enable specified interrupt in NVIC
  NVIC_InitTypeDef init_struct = {
    .NVIC_IRQChannel = irq_channel,
    .NVIC_IRQChannelPreemptionPriority = priority,
    .NVIC_IRQChannelCmd = ENABLE,
  };
  NVIC_Init(&init_struct);
  return STATUS_CODE_OK;
}

StatusCode stm32f10x_interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings) {
  if (!settings || settings->type >= NUM_INTERRUPT_CLASSES ||
      settings->edge >= NUM_INTERRUPT_EDGES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Enable exti channel specified by pin number at specified port
  GPIO_EXTILineConfig(address->port, address->pin);
  EXTI_InitTypeDef init_struct = {
    .EXTI_Line = EXTI_LINE(address->pin),
    .EXTI_Mode = EXTI_MODE(settings->type),
    .EXTI_Trigger = EXTI_TRIGGER(settings->edge),
    .EXTI_LineCmd = ENABLE,
  };
  EXTI_Init(&init_struct);
  
  // Enable interrupt at that port
  status_ok_or_return(stm32f10x_interrupt_nvic_enable(prv_get_irq_channel(address->pin), settings->priority));
  return STATUS_CODE_OK;
}

StatusCode stm32f10x_interrupt_exti_trigger(uint8_t line) {
  if (line > NUM_STM32F10X_EXTI_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Trigger exti channel response
  EXTI_GenerateSWInterrupt(EXTI_LINE(line));
  return STATUS_CODE_OK;
}

StatusCode stm32f10x_interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  if (line >= NUM_STM32F10X_INTERRUPT_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *pending_bit = (uint8_t)EXTI_GetITStatus(EXTI_LINE(line));
  return STATUS_CODE_OK;
}

StatusCode stm32f10x_interrupt_exti_clear_pending(uint8_t line) {
  if (line >= NUM_STM32F10X_INTERRUPT_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_ClearITPendingBit(EXTI_LINE(line));
  return STATUS_CODE_OK;
}

StatusCode stm32f10x_interrupt_exti_mask_set(uint8_t line, bool masked) {
  if (line >= NUM_STM32F10X_INTERRUPT_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  
  if (masked) {
    EXTI->IMR &= ~(EXTI_LINE(line));
  } else {
    EXTI->IMR |= EXTI_LINE(line);
  }

  return STATUS_CODE_OK;
}
