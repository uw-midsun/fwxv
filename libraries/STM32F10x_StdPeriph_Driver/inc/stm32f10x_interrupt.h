#pragma once

// Midsun Firmware implementation to overlay interactions with STM32f1 interrupts 
// The STM32F103C8T6 (under the category of STM32F10x Medium Density) provides
// 43 Maskable interrupt channels and 16 priority levels
//
// This API provides methods to enable interrupts in the NVIC, as well as configure
// external interrupts (gpio interrupts)

#include <stdint.h>
#include <stdbool.h>

#include "interrupt_def.h"
#include "stm32f10x_exti.h"
#include "status.h"
#include "gpio.h"

// From the User Manual.
// Indexed STM32F10X_MD NVIC entries, from IRQn_Type enum in stm32f10x.h
#define NUM_STM32F10X_INTERRUPT_CHANNELS 43

// Number of available external input lines
#define NUM_STM32F10X_EXTI_LINES 16

// Initializes interrupt internals. If called multiple times the subsequent attempts will clear
// everything resulting in needing to re initialize al interrupts.
void stm32f10x_interrupt_init(void);

// Enables the nested interrupt vector controller for a given channel.
StatusCode stm32f10x_interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority);

// Enables an external interrupt line with the given settings.
// STM32F1 defines 16 possible lines for use, defined in stm32f10x_exti.h
StatusCode stm32f10x_interrupt_exti_enable(GpioAddress *address, const InterruptSettings *settings);

// Triggers a software interrupt on a given external interrupt channel.
StatusCode stm32f10x_interrupt_exti_trigger(uint8_t line);

// Get the pending flag for an external interrupt.
StatusCode stm32f10x_interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit);

// Clears the pending flag for an external interrupt.
StatusCode stm32f10x_interrupt_exti_clear_pending(uint8_t line);

// Masks or clears the external interrupt on the given line.
StatusCode stm32f10x_interrupt_exti_mask_set(uint8_t line, bool masked);
