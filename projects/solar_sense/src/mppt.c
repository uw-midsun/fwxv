#include "mppt.h"

#include "spi.h"

// MPPT ports
#define MPPT_GPIO_PORT GPIO_PORT_B
#define MPPT_SPI_PORT SPI_PORT_1

// MPPT commands
#define MPPT_TURN_ON_CMD 0x3
#define MPPT_READ_CURRENT_CMD 0x4
#define MPPT_READ_VIN_CMD 0x5
#define MPPT_READ_PWM_CMD 0x6
#define MPPT_READ_STATUS_CMD 0x7

StatusCode mppt_init(uint8_t mppt_num) {
  const SpiSettings spi_settings = {
    .baudrate = 1000000,
    .mode = SPI_MODE_3,
    .mosi = { .port = MPPT_GPIO_PORT, .pin = 15 },
    .miso = { .port = MPPT_GPIO_PORT, .pin = 14 },
    .sclk = { .port = MPPT_GPIO_PORT, .pin = 13 },
    .cs = { .port = MPPT_GPIO_PORT, .pin = 12 },
  };

  const uint8_t mppt_turn_on_cmd = MPPT_TURN_ON_CMD;

  // Turning on MPPT
  status_ok_or_return(spi_init(MPPT_SPI_PORT, &spi_settings));
  status_ok_or_return(spi_cs_set_state(MPPT_SPI_PORT, GPIO_STATE_LOW));
  status_ok_or_return(spi_tx(MPPT_SPI_PORT, &mppt_turn_on_cmd, 1));
  status_ok_or_return(spi_cs_set_state(MPPT_SPI_PORT, GPIO_STATE_HIGH));

  return STATUS_CODE_OK;
}

StatusCode mppt_read_current(SpiPort MPPT_SPI_PORT, uint16_t *current_data) {
  const uint8_t mppt_read_current_cmd = MPPT_READ_CURRENT_CMD;
  status_ok_or_return(
      spi_exchange(MPPT_SPI_PORT, &mppt_read_current_cmd, 1, (uint8_t *)current_data, 2));
  return STATUS_CODE_OK;
}

StatusCode mppt_read_voltage(SpiPort MPPT_SPI_PORT, uint16_t *voltage_data) {
  const uint8_t mppt_read_vin_cmd = MPPT_READ_VIN_CMD;
  status_ok_or_return(
      spi_exchange(MPPT_SPI_PORT, &mppt_read_vin_cmd, 1, (uint8_t *)voltage_data, 2));
  return STATUS_CODE_OK;
}
StatusCode mppt_read_pwm(SpiPort MPPT_SPI_PORT, uint16_t *pwm_data) {
  const uint8_t mppt_read_pwm_cmd = MPPT_READ_PWM_CMD;
  status_ok_or_return(spi_exchange(MPPT_SPI_PORT, &mppt_read_pwm_cmd, 1, (uint8_t *)pwm_data, 2));
  return STATUS_CODE_OK;
}
StatusCode mppt_read_status(SpiPort MPPT_SPI_PORT, uint16_t *status_data) {
  const int8_t mppt_read_status_cmd = MPPT_READ_STATUS_CMD;
  status_ok_or_return(
      spi_exchange(MPPT_SPI_PORT, &mppt_read_status_cmd, 1, (uint8_t *)status_data, 2));
  return STATUS_CODE_OK;
}

static StatusCode prv_select_mppt(uint8_t mppt_num) {}
