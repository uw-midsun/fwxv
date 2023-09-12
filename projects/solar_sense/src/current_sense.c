#include "current_sense.h"

const GpioAddress gpio_addr_relay_status = { .port = GPIO_PORT_A, .pin = 6 };
const GpioAddress gpio_addr_relay_enable = { .port = GPIO_PORT_B, .pin = 4 };

void current_sense_init() {
  // Initializes I2C for ADC readings of current
  I2CSettings i2c_current_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 8 },
    .sda = { .port = GPIO_PORT_B, .pin = 9 },
  };
  i2c_init(I2C_PORT_2, &i2c_current_settings);

  // Initialize I2c for Voltage reading
  I2CSettings i2c_voltage_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };
  i2c_init(I2C_PORT_2, &i2c_voltage_settings);
}

StatusCode read_relay_status(uint8_t *relay_status) {
  LOG_DEBUG("solar sense relay status\n");
  return STATUS_CODE_OK;
}
StatusCode read_voltage(uint16_t *voltage) {
  LOG_DEBUG("solar sense voltage\n");
  return STATUS_CODE_OK;
}
StatusCode read_current_sense(uint16_t *current) {
  LOG_DEBUG("Current sense\n");
  return STATUS_CODE_OK;
}
StatusCode relay_fault() {
  LOG_DEBUG("Solar Sense Fault! \n");
  return STATUS_CODE_OK;
}
StatusCode solar_sense_relay_close() {
  LOG_DEBUG("Solar sense relay close\n");
  return STATUS_CODE_OK;
}
StatusCode solar_sense_relay_open() {
  LOG_DEBUG("Solar sense relay open\n");
  return STATUS_CODE_OK;
}
