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
  cmd = 0x8084; /* TODO: make setting this to continuous conversion an onboarding task */
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  /* TODO make this onboarding task*/
  // Set low thresh to zero
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  /* TODO make this onboarding task*/
  // Set high thresh to 1V
  cmd = 1 / (6.25E-5);
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  // Register the ALRT pin
  /* TODO make filling out the settings an onboarding task */
  const InterruptSettings settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_FALLING,
  };
  gpio_it_register_interrupt(config->ready_pin, &settings, ALRT_EVENT, config->handler_task);
  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t cmd;

  // Write Config register
  cmd = 0x8084; /* TODO: make setting this to continuous conversion an onboarding task */
  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, int16_t *reading) {
  status_ok_or_return(i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION,
                                   (uint8_t *)reading, 2));
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  int16_t tmp;
  status_ok_or_return(ads1115_read_raw(config, channel, &tmp));
  *reading = tmp * (6.25E-5);
  return STATUS_CODE_OK;
}
