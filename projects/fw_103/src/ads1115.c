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
  cmd = 0x0000;

  // Configure for continuous mode (MODE bit = 0)
  cmd |= (0 << 15);      // OS: No effect in continuous mode
  cmd |= (0b000 << 12);  // MUX: AIN0-AIN1 differential (default)
  cmd |= (0b010 << 9);   // PGA: ±2.048V range (default)
  cmd |= (0 << 8);       // MODE: Continuous mode (changed from default single-shot)
  cmd |= (0b100 << 5);   // DR: 128 SPS (default)
  cmd |= (0 << 4);       // COMP_MODE: Traditional comparator (default)
  cmd |= (0 << 3);       // COMP_POL: Active low (default)
  cmd |= (0 << 2);       // COMP_LAT: Non-latching (default)
  cmd |= (0b11 << 0);    // COMP_QUE: Disable comparator (default)

  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);

  // Set low threshold to 0V
  cmd = 0x0000;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_LO_THRESH, (uint8_t *)(&cmd), 2);

  // Set high threshold to 1V
  cmd = 0x0000;
  float voltage = 1.0;
  int16_t signed_val = (int16_t)((voltage / 2.048) * 32767);
  cmd = (uint16_t)signed_val;
  i2c_write_reg(config->i2c_port, i2c_addr, ADS1115_REG_HI_THRESH, (uint8_t *)(&cmd), 2);

  // Register the ALRT pin interrupt
  if (config->ready_pin != NULL && config->handler_task != NULL) {
    gpio_init_pin(config->ready_pin, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);

    InterruptSettings settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,
      .priority = INTERRUPT_PRIORITY_NORMAL,
      .edge = INTERRUPT_EDGE_FALLING,
    };

    gpio_it_register_interrupt(config->ready_pin, &settings, ALRT_EVENT, config->handler_task);
  }

  return STATUS_CODE_OK;
}

StatusCode ads1115_select_channel(ADS1115_Config *config, ADS1115_Channel channel) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t cmd;
  cmd = 0x0000;

  i2c_write_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONFIG, (uint8_t *)(&cmd), 2);
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_raw(ADS1115_Config *config, ADS1115_Channel channel, uint16_t *reading) {
  if (config == NULL || reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  ads1115_select_channel(config, channel);

  i2c_read_reg(config->i2c_port, config->i2c_addr, ADS1115_REG_CONVERSION, (uint8_t *)reading,
               sizeof(uint16_t));
  return STATUS_CODE_OK;
}

StatusCode ads1115_read_converted(ADS1115_Config *config, ADS1115_Channel channel, float *reading) {
  if (config == NULL || reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t raw;
  StatusCode status = ads1115_read_raw(config, channel, &raw);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  int16_t raw_signed = (int16_t)raw;

  float volts = ((float)raw_signed / 32767.0f) * 2.048f;
  *reading = volts;

  return STATUS_CODE_OK;
}