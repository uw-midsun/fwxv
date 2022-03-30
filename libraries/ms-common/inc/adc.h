#pragma once
// Analog to Digital Converter HAL Inteface
// Requires GPIO and interrupts to be initialized.
// If using continuous mode on x86, soft timers must be initialized.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
  NUM_ADC_MODES,
} AdcMode;

// Initialize the ADC to the desired conversion mode
void adc_init(AdcMode adc_mode);

// Enable or disable a given pin.
// A race condition may occur when setting a pin during a conversion.
// However, it should not cause issues given the intended use cases
// To set adc channels REF/TEMP/BAT, you must use adc_set_channel() below
StatusCode adc_set_channel_pin(GpioAddress address, bool new_state);

// Register a callback function to be called when the specified pin
// completes a conversion
StatusCode adc_register_callback_pin(GpioAddress address, AdcPinCallback callback, void *context);

// Do not call |adc_read_raw/converted| or |adc_read_raw/converted_pin| from an interrupt callback
// with INTERRUPT_PRIORITY_HIGH, as it will cause deadlock.

// Obtain the raw 12-bit value read by the specified pin
StatusCode adc_read_raw_pin(GpioAddress address, uint16_t *reading);

// Obtain the converted value at the specified pin, in mV
StatusCode adc_read_converted_pin(GpioAddress address, uint16_t *reading);
