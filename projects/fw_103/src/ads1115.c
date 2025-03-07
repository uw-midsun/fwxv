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
  cmd = 0b0000010010000011; // 0 000 010 0 100 0 0 0 11, last 00 for alrt pin, 11 for disable compartor to thresholds
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set low thresh to zero
  cmd = 0x0000; //signed bit so i can have positive max be 2.048
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* TODO (optional) */
  // Set high thresh to 1V
  cmd = 0b11111001111111; //32767*1/2.048 = 15999
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_HI_THRESH, (uint8_t *)(&cmd), 2);

  // Register the ALRT pin, would be anything but 11 in config register
  /* TODO (optional) */
  gpio_init_pin(ready_pin, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH); // line is active low

  InterruptSettings settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .edge = INTERRUPT_EDGE_FALLING,
  };
  
  gpio_it_register_interrupt(ready_pin, &settings, 1, config->handler_task);

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
  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, reading, 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  /* TODO: complete function */
  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, reading, 2);
  // twos complement
  *reading = *reading/32767.0*2.048; // -2.048 to 2.048 V
  return STATUS_CODE_OK;
}
