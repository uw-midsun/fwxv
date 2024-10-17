#include "ads1115.h"

#include "gpio_it.h"
#include "i2c.h"
#include "status.h"

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  config->i2c_addr = i2c_addr;

  uint16_t cmd;

  // Write Config register
  /* TODO: fill out this value */
  cmd = 0x8583;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set low thresh to zero
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set high thresh to 1V
  cmd = 16000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  // Register the ALRT pin
  /* TODO (optional) */

  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t cmd;

  // Write Config register
  cmd = 0x8583 | (channel << 12);
  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading) {
  /* TODO: complete function */
  if (config == NULL || reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Select the channel
  StatusCode status = ads1115_select_channel(config, channel);
  if (!status_ok(status)) {
    return status;
  }

  // Read the conversion register
  uint8_t data[2];
  status = i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, data, 2);
  if (!status_ok(status)) {
    return status;
  }

  *reading = (data[0] << 8) | data[1];
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* TODO: complete function */
  if (config == NULL || reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t raw_reading;
  StatusCode status = ads1115_read_raw(config, channel, &raw_reading);
  if (!status_ok(status)) {
    return status;
  }

  // Convert raw reading to voltage
  // Assuming the reference voltage is 2.048V and the ADC is 16-bit
  *reading = (raw_reading / 32767.0) * 2.048;

  return STATUS_CODE_OK;
}
