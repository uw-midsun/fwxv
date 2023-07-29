#include "mppt.h"

#include "log.h"

void mppt_init() {
  // Initializes ADC for ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
}

StatusCode mppt_shut(SpiPort port, GpioAddress pin) {
  LOG_DEBUG("mppt shut\n");
  return STATUS_CODE_OK;
}

StatusCode mppt_turn_on(SpiPort port, GpioAddress pin) {
  LOG_DEBUG("mppt turn on\n");
  return STATUS_CODE_OK;
}
StatusCode mppt_read_current(SpiPort port, uint16_t *current, GpioAddress pin) {
  LOG_DEBUG("mppt read current\n");
  return STATUS_CODE_OK;
}
// StatusCode mppt_read_voltage_in(SpiPort port, uint16_t* vin, GpioAddress pin);
StatusCode mppt_read_pwm(SpiPort port, uint16_t *pwm, GpioAddress pin) {
  LOG_DEBUG("mppt read pwm\n");
  return STATUS_CODE_OK;
}
StatusCode mppt_read_status(SpiPort port, uint8_t *status, GpioAddress pin) {
  LOG_DEBUG("mppt read status\n");
  return STATUS_CODE_OK;
}
