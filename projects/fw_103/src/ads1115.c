#include "ads1115.h"

#include "gpio_it.h"
#include "i2c.h"
#include "status.h"

// Completed by Ethan Gan 
StatusCode ads1115_init(ADS1115_Config *config, ADS1115_Address i2c_addr, GpioAddress *ready_pin) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  config->i2c_addr = i2c_addr;

  uint16_t cmd;

  // Write Config register
  /* TODO: fill out this value */
  cmd = 0x0483;

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
  cmd = 0x0483;
  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading) {
  /* TODO: complete function */
  // i2c already initialized somewhere else
  uint16_t cmd = 0x0483;
  // read 2 bytes
    StatusCode status =  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading, 2);
    if(status!=STATUS_CODE_OK) {
    return status;
  }
    // this info is stored in reading
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* TODO: complete function */
  uint16_t raw_reading;
  uint16_t cmd = 0x0483;
  // the reason why it is mapped to int8_t other than the header file calling for it
  // is because the rx_data really only cares about pointing to the start of the memory address to deal with the data byte by byte which is why we enter 2 bytes as rx_len.
  StatusCode status = i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)&raw_reading, 2);
  
  if(status!=STATUS_CODE_OK) {
    return status;
  }

  //voltage = (raw/max)*range - (max magnitude)
  // remember to avoid integer division
  *reading = ((raw_reading/65535.0f)*4.096f)-2.048; 
  return STATUS_CODE_OK;
}
