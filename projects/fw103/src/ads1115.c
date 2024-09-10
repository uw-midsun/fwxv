#include "ads1115.h"

#include "gpio_it.h"
#include "i2c.h"
#include "status.h"
#include "log.h"

StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  LOG_DEBUG("Entered ads1115 init\n");
  config->i2c_addr = i2c_addr;

  uint16_t cmd;

  // Write Config register
  /* TODO: fill out this value */
  cmd = 0x0483;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set low thresh to zero
  // cmd = 0x0000;
  // i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set high thresh to 1V
  // cmd = 0x7C1El;
  // i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_HI_THRESH, (uint8_t *)(&cmd), 2);

  // Register the ALRT pin
  /* TODO (optional) */

  LOG_DEBUG("Exited ads1115 init\n");
  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {

  uint16_t cmd;

  // Write Config register
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {

  // Read 2 bytes from the ADS1115 conversion register
  StatusCode status = i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading, 2);

  return STATUS_CODE_OK;  // Return OK if everything is successful
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {

  int16_t raw_adc_value;

  ads1115_read_raw(config, channel, &raw_adc_value);

  // Convert raw ADC value to voltage (assuming range of 0 to 2.048V)
  *reading = (float)raw_adc_value / 65535.0f * 2.048f;

  return STATUS_CODE_OK;  // Return OK if conversion is successful
}
