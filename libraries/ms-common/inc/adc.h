#pragma once
// Analog to Digital Converter HAL Inteface
// Requires GPIO and interrupts to be initialized.
// If using continuous mode on x86, soft timers must be initialized.
//
// To use the adc, initialize all needed pins, both as GPIOs with requisite settings
// and as analog inputs:
//
//    GpioAddress my_addr = {.port = PORT, .pin = PIN};
//    gpio_init_pin(&my_addr, GPIO_ANALOG, GPIO_STATE_LOW);
//
//    adc_add_channel(my_addr);
//    ...
//    adc_init(ADC_MODE);
//
// Pins must be added as channels before adc init is called.
// Adding pins creates a sequential list needed to match readings
// with addresses. It can not be changed after initialization.
//
// To get a reading for a specified pin:
//
//    adc_read_converted(my_addr, &reading);
// or
//    adc_read_raw(my_addr, &reading);
//
// In Single mode, this will trigger and wait on one conversion
// In Continuous mode, this will read the most recent data
// If ADC_REF or ADC_TEMP is passed as the gpio address, the
// value for this will be from the vref/internal temp sensor

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "notify.h"
#include "status.h"
#include "tasks.h"

// Total Number of ADC channels (16 external, 2 internal)
#define NUM_ADC_CHANNELS 18

#define ADC_TIMEOUT_MS 100

typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
  NUM_ADC_MODES,
} AdcMode;

extern const GpioAddress ADC_REF;
extern const GpioAddress ADC_TEMP;

// Initialize the ADC to the desired conversion mode
StatusCode adc_init(AdcMode adc_mode);

// Configures a GPIO to be used as an ADC channel
// Must be called for all pins in use before adc_init()
StatusCode adc_add_channel(GpioAddress address);

// Obtain the raw 12-bit value read by the specified pin
StatusCode adc_read_raw(GpioAddress address, uint16_t *reading);

// Obtain the converted value at the specified pin, in mV
StatusCode adc_read_converted(GpioAddress address, uint16_t *reading);

// Used to clear state
void adc_deinit(void);

// Setter for adc testing
#ifdef MS_PLATFORM_X86
void adc_set_reading(GpioAddress sample_address, uint16_t adc_reading);
#endif
