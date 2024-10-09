#include "ads1115.h"
#include "gpio_it.h"
#include "i2c.h"
#include "status.h"
#include "log.h"
StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  config->i2c_addr = i2c_addr;

  uint16_t cmd;

  // Write Config register
  /* TODO: fill out this value */
  cmd = 0x0483; //in hexadecimal, representing the bit confirguration 
  //configuring it correctly when it starts up
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
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2); //why rewriting?????
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading) {
  uint8_t raw_value[2];
  StatusCode status = i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, raw_value, 2); //raw value decays to pointer to array

  if (status != STATUS_CODE_OK) {
    return status; 
  }
  
  *reading = (raw_value[0] << 8) | raw_value[1];  //accessing byte 1, then combine it with second byte to get full 16 bits
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) { //reads from the ADC, which we have already confirugred
  uint16_t raw_adc_value;
  const float full_scale_voltage = 4.096; 
  StatusCode status = ads1115_read_raw(config, channel, &raw_adc_value); //the function it calls effectively changes the raw_adc_value 

  if (status != STATUS_CODE_OK) {
    return status; 
  }

  *reading = (raw_adc_value / 65535.0) * full_scale_voltage - 2.048; //conversion

  return STATUS_CODE_OK;
}
