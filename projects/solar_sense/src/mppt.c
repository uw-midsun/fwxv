#include "mppt.h"

#include "spi.h"

// MPPT commands
#define MPPT_TURN_ON_CMD 0x3
#define MPPT_READ_CURRENT_CMD 0x4
#define MPPT_READ_VIN_CMD 0x5
#define MPPT_READ_PWM_CMD 0x6
#define MPPT_READ_STATUS_CMD 0x7

StatusCode mppt_init(SpiPort spi_port, uint8_t gpio_port) {
  const SpiSettings spi_settings = {
    .baudrate = 1000000,
    .mode = SPI_MODE_3,
    .mosi = { .port = gpio_port, .pin = 7 },
    .miso = { .port = gpio_port, .pin = 6 },
    .sclk = { .port = gpio_port, .pin = 5 },
    .cs = { .port = gpio_port, .pin = 4 },
  };

  uint8_t mppt_turn_on_cmd = MPPT_TURN_ON_CMD;

  // Turning on MPPT
  status_ok_or_return(spi_init(spi_port, &spi_settings));
  status_ok_or_return(spi_cs_set_state(spi_port, GPIO_STATE_LOW));
  status_ok_or_return(spi_tx(spi_port, &mppt_turn_on_cmd, 1));
  status_ok_or_return(spi_cs_set_state(spi_port, GPIO_STATE_HIGH));

  return STATUS_CODE_OK;
}

StatusCode mppt_read_current(SpiPort spi_port, uint16_t *current_data) {
  uint8_t mppt_read_current_cmd = MPPT_READ_CURRENT_CMD;
  status_ok_or_return(
      spi_exchange(spi_port, &mppt_read_current_cmd, 1, (uint8_t *)current_data, 2));
  return STATUS_CODE_OK;
}

StatusCode mppt_read_voltage(SpiPort spi_port, uint16_t *voltage_data) {
  uint8_t mppt_read_vin_cmd = MPPT_READ_VIN_CMD;
  status_ok_or_return(spi_exchange(spi_port, &mppt_read_vin_cmd, 1, (uint8_t *)voltage_data, 2));
  return STATUS_CODE_OK;
}
StatusCode mppt_read_pwm(SpiPort spi_port, uint16_t *pwm_data) {
  uint8_t mppt_read_pwm_cmd = MPPT_READ_PWM_CMD;
  status_ok_or_return(spi_exchange(spi_port, &mppt_read_pwm_cmd, 1, (uint8_t *)pwm_data, 2));
  return STATUS_CODE_OK;
}
StatusCode mppt_read_status(SpiPort spi_port, uint16_t *status_data) {
  uint8_t mppt_read_status_cmd = MPPT_READ_STATUS_CMD;
  status_ok_or_return(spi_exchange(spi_port, &mppt_read_status_cmd, 1, (uint8_t *)status_data, 2));
  return STATUS_CODE_OK;
}
