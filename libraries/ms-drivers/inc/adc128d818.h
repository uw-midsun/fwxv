// adc128d818.h
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "gpio.h"
#include "i2c.h"

// Enum for ADC128D818 status codes
typedef enum {
  ADC128D818_STATUS_CODE_OK = 0,
  ADC128D818_STATUS_CODE_ERROR,
  NUM_ADC128D818_STATUS_CODES,
} Adc128d818StatusCode;

// Callback type for error handling
typedef void (*Adc128d818ErrorHandlerCb)(Adc128d818StatusCode code, void *context);

// Struct for ADC128D818 settings
typedef struct {
  I2CPort i2c_port;
  uint8_t i2c_address;
  GpioAddress alert_pin;
  Adc128d818ErrorHandlerCb error_handler;
  void *error_context;
} Adc128d818Settings;

// Struct for ADC128D818 storage
typedef struct {
  I2CPort i2c_port;
  uint8_t i2c_address;
  Adc128d818ErrorHandlerCb error_handler;
  void *error_context;
} Adc128d818Storage;

// Initializes the ADC128D818 with the given settings
StatusCode adc128d818_init(Adc128d818Storage *storage, const Adc128d818Settings *settings);

// Reads a channel value from the ADC128D818
StatusCode adc128d818_read_channel(Adc128d818Storage *storage, uint8_t channel, uint16_t *reading);

// Sets the configuration register of the ADC128D818
StatusCode adc128d818_set_config(Adc128d818Storage *storage, uint8_t config);

// Sets the conversion rate of the ADC128D818
StatusCode adc128d818_set_conversion_rate(Adc128d818Storage *storage, uint8_t rate);

// Enables or disables a channel on the ADC128D818
StatusCode adc128d818_set_channel_enable(Adc128d818Storage *storage, uint8_t channel, bool enable);

// Start a single-shot conversion
StatusCode adc128d818_start_one_shot(Adc128d818Storage *storage);