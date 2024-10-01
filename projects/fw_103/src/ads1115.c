#include "ads1115.h"

#include "gpio.h"
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
  cmd = 0x0000 + (0b010 << 9) + (0b100 << 5) + 0b11;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set low thresh to zero
  cmd = 32768;  // -2.048 + 2.048 V = 0V
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set high thresh to 1V
  cmd = 48768;  // -2.048 + 3.048 V = 1V
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_HI_THRESH, (uint8_t *)(&cmd), 2);

  // Register the ALRT pin
  /* TODO (optional) */
  gpio_init_pin(config->ready_pin, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);

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
  return i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION,
                      (uint8_t *)reading, sizeof(uint16_t));
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  uint16_t result;
  StatusCode res = ads1115_read_raw(config, channel, &result);
  if (res != STATUS_CODE_OK) return res;
  *reading = -2.048 + (float)result / 65535 * 4.096;
  return STATUS_CODE_OK;
}
