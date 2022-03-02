#pragma once
// GPIO Interrupt Handlers
// Requires GPIO and interrupts to be initialized.
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "gpio.h"
#include "interrupt_def.h"
#include "status.h"

// Initializes the interrupt handler for GPIO.
void gpio_it_init(void);

// Gets the interrupt edge the interrupt is set to trigger on
StatusCode gpio_it_get_edge(const GpioAddress *address, InterruptEdge *edge);

// Registers a new callback on a given port pin combination with the desired
// settings. Set the notify value bit when a gpio_it occurs
StatusCode gpio_it_register_interrupt(const GpioAddress *address, const InterruptSettings *settings,
                                      InterruptEdge edge, TaskHandle_t task_to_notify,
                                      uint8_t bit_to_set);

// Triggers an interrupt in software.
StatusCode gpio_it_trigger_interrupt(const GpioAddress *address);

// Masks the interrupt for the given address if masked is True.
// Enables the interrupt if masked is false.
StatusCode gpio_it_mask_interrupt(const GpioAddress *address, bool masked);
