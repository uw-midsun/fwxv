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

  // Bit #: | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |     4     |     3    |    2     |  1  |  0  |
  // Field: | OS |      MUX     |     PGA      |MODE|       DR     | COMP_MODE | COMP_POL | COMP_LAT |  COMP_QUE |
  // Value: | 1  |      000     |     010      | 0  |      100     |     0     |     0    |     0    |     11    |

  // Write Config register
  /* TODO: fill out this value */
  cmd = 0x8483; // This should be the hex code for that binary value
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set low thresh to zero
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set high thresh to 1V
  cmd = 0x0000;
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
  cmd = 0x8483; //I'm assuming the same configruation as earlier should be written here. I feel like it might be wrong
  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  /* TODO: complete function */
  StatusCode status = ads1115_select_channel(config, channel);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading, 2); //
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* TODO: complete function */
  int16_t raw_reading;
  StatusCode status = ads1115_read_raw(config, channel, &raw_reading);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  *reading = raw_reading * (2.048 / 65535);
  return STATUS_CODE_OK;
}
