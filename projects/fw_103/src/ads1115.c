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
  cmd = 0x8483; // 0b1 000 010 0 100 0 0 0 11
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set low thresh to zero
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set high thresh to 1V
  // The range of the programmable gain amplifier is +/- 2.048, so it has a total range of 4.096
  // The ADS1115 has 16-bit resolution, so size of each LSB found via 4.096/2^16 = 0.0000625 -> 0.0000625V/step
  // To get 1V, divide 1V by calc'd value, 1V/(0.0000625V/step) = 16,000 (decimal)
  // Hex of 16000 -> 0x3E80
  cmd = 0x3E80;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_HI_THRESH, (uint8_t *)(&cmd), 2);

  // Register the ALRT pin
  /* TODO (optional) */
  gpio_init_pin(ready_pin, GPIO_INPUT_PULL_UP, GPIO_STATE_LOW); // needs to be pull up according to 7.3.8

  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t cmd;

  // Write Config register
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading) {
  /* TODO: complete function */
  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading, 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* TODO: complete function */
  uint16_t raw_reading;
  ads1115_read_raw(config, channel, &raw_reading);
  *reading = (raw_reading / 65535.0) - 2.048;
  return STATUS_CODE_OK;
}
