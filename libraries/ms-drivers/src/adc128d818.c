#include "adc128d818.h"
#include "adc128d818_def.h"
#include "gpio.h"
#include "i2c.h"
#include "status.h"
#include <string.h>

// Helper function to write to a register
static StatusCode prv_write_register(Adc128d818Storage *storage, uint8_t reg, uint8_t value) {
  uint8_t data[2] = { reg, value };
  return i2c_write(storage->i2c_port, storage->i2c_address, data, sizeof(data));
}

// Helper function to read from a register
static StatusCode prv_read_register(Adc128d818Storage *storage, uint8_t reg, uint8_t *value) {
  status_ok_or_return(i2c_write(storage->i2c_port, storage->i2c_address, &reg, 1));
  return i2c_read(storage->i2c_port, storage->i2c_address, value, 1);
}

// Initializes the ADC128D818 with the given settings
StatusCode adc128d818_init(Adc128d818Storage *storage, const Adc128d818Settings *settings) {
  if (storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(Adc128d818Storage));
  storage->i2c_port = settings->i2c_port;
  storage->i2c_address = settings->i2c_address;
  storage->error_handler = settings->error_handler;
  storage->error_context = settings->error_context;

  // Set default configuration
  status_ok_or_return(prv_write_register(storage, ADC128D818_REG_CONFIG, ADC128D818_DEFAULT_CONFIG));
  status_ok_or_return(prv_write_register(storage, ADC128D818_REG_ADV_CONFIG, ADC128D818_DEFAULT_ADV_CONFIG));

  return STATUS_CODE_OK;
}

// Reads a channel value from the ADC128D818
StatusCode adc128d818_read_channel(Adc128d818Storage *storage, uint8_t channel, uint16_t *reading) {
  if (channel >= 8 || storage == NULL || reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t reg = ADC128D818_REG_CHANNEL_READINGS_START + channel;
  uint8_t data[2] = { 0 };

  status_ok_or_return(i2c_write(storage->i2c_port, storage->i2c_address, &reg, 1));
  status_ok_or_return(i2c_read(storage->i2c_port, storage->i2c_address, data, sizeof(data)));

  *reading = ((uint16_t)data[0] << 8) | data[1];
  return STATUS_CODE_OK;
}

// Sets the configuration register of the ADC128D818
StatusCode adc128d818_set_config(Adc128d818Storage *storage, uint8_t config) {
  return prv_write_register(storage, ADC128D818_REG_CONFIG, config);
}

// Sets the conversion rate of the ADC128D818
StatusCode adc128d818_set_conversion_rate(Adc128d818Storage *storage, uint8_t rate) {
  return prv_write_register(storage, ADC128D818_REG_CONV_RATE, rate);
}

// Enables or disables a channel on the ADC128D818
StatusCode adc128d818_set_channel_enable(Adc128d818Storage *storage, uint8_t channel, bool enable) {
  if (channel >= 8) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t reg_value = 0;
  status_ok_or_return(prv_read_register(storage, ADC128D818_REG_CHANNEL_DISABLE, &reg_value));

  if (enable) {
    reg_value &= ~(1 << channel);
  } else {
    reg_value |= (1 << channel);
  }

  return prv_write_register(storage, ADC128D818_REG_CHANNEL_DISABLE, reg_value);
}

// Start a single-shot conversion
StatusCode adc128d818_start_one_shot(Adc128d818Storage *storage) {
  return prv_write_register(storage, ADC128D818_REG_ONE_SHOT, 1);
}