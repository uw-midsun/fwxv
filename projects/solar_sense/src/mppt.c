#define NUM_MPPTS 6

#include "mppt.h"
#include "solar_sense_setters.h"

static SpiPort mppt_spi_port;

// MPPT commands
#define MPPT_TURN_ON_CMD 0x3
#define MPPT_READ_CURRENT_CMD 0x4
#define MPPT_READ_VIN_CMD 0x5
#define MPPT_READ_PWM_CMD 0x6
#define MPPT_READ_STATUS_CMD 0x7

// MPPT chip select GPIOS
static const GpioAddress MPPT_CS_GPIO_PINS[] = {
  { GPIO_PORT_B, 0 },
  { GPIO_PORT_B, 1 },
  { GPIO_PORT_B, 2 },
};

static StatusCode prv_select_mppt(const uint8_t mppt_idx) {
  GpioState PB0_state = mppt_idx & 0x1;
  GpioState PB1_state = (mppt_idx & 0x2) >> 1;
  GpioState PB2_state = (mppt_idx & 0x4) >> 2;

  status_ok_or_return(gpio_set_state(&MPPT_CS_GPIO_PINS[0], PB0_state));
  status_ok_or_return(gpio_set_state(&MPPT_CS_GPIO_PINS[1], PB1_state));
  status_ok_or_return(gpio_set_state(&MPPT_CS_GPIO_PINS[2], PB2_state));

  return STATUS_CODE_OK;
}

StatusCode mppt_init(const SpiSettings *settings, const SpiPort port) {
  mppt_spi_port = port;

  // Initialize MPPT chip select GPIOS
  for (uint8_t i = 0; i < sizeof(MPPT_CS_GPIO_PINS) / sizeof(GpioAddress); i++) {
      gpio_init_pin(&MPPT_CS_GPIO_PINS[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  }

  status_ok_or_return(spi_init(mppt_spi_port, settings));

  const uint8_t mppt_turn_on_cmd = MPPT_TURN_ON_CMD;

  for (uint8_t i = 0; i < NUM_MPPTS; i++) {
    // Selecting correct MPPT
    status_ok_or_return(prv_select_mppt(i));

    // Turning on MPPT
    status_ok_or_return(spi_cs_set_state(mppt_spi_port, GPIO_STATE_LOW));
    status_ok_or_return(spi_tx(mppt_spi_port, &mppt_turn_on_cmd, 1));
    status_ok_or_return(spi_cs_set_state(mppt_spi_port, GPIO_STATE_HIGH));
  }

  return STATUS_CODE_OK;
}

StatusCode mppt_read_current(uint16_t *current_data) {
  const uint8_t mppt_read_current_cmd = MPPT_READ_CURRENT_CMD;
  status_ok_or_return(
    spi_exchange(mppt_spi_port, &mppt_read_current_cmd, 1, (uint8_t *)current_data, 2));
  return STATUS_CODE_OK;
}

StatusCode mppt_read_voltage(uint16_t *voltage_data) {
  const uint8_t mppt_read_vin_cmd = MPPT_READ_VIN_CMD;
  status_ok_or_return(
    spi_exchange(mppt_spi_port, &mppt_read_vin_cmd, 1, (uint8_t *)voltage_data, 2));
  return STATUS_CODE_OK;
}

StatusCode mppt_read_pwm(uint16_t *pwm_data) {
  const uint8_t mppt_read_pwm_cmd = MPPT_READ_PWM_CMD;
  status_ok_or_return(
    spi_exchange(mppt_spi_port, &mppt_read_pwm_cmd, 1, (uint8_t *)pwm_data, 2));
  return STATUS_CODE_OK;
}

StatusCode mppt_read_status(uint16_t *status_data) {
  const int8_t mppt_read_status_cmd = MPPT_READ_STATUS_CMD;
  status_ok_or_return(
    spi_exchange(mppt_spi_port, &mppt_read_status_cmd, 1, (uint8_t *)status_data, 2));
  return STATUS_CODE_OK;
}

// Reads data from all MPPTs and sets the appropriate CAN messages
StatusCode read_mppts() {
  uint16_t current_data, voltage_data, pwm_data, status_data = 0;

  // Reads data from all MPPTs
  for (uint8_t i = 0; i < NUM_MPPTS; i++) {
    // Selects correct MPPT
    status_ok_or_return(prv_select_mppt(i));

    // Reads data
    status_ok_or_return(mppt_read_current(&current_data));
    status_ok_or_return(mppt_read_voltage(&voltage_data));
    status_ok_or_return(mppt_read_voltage(&pwm_data));
    status_ok_or_return(mppt_read_status(&status_data));

    // Sets the appropriate CAN messages
    switch (i) {
      case 1:
        set_mppt_1_current(current_data);
        set_mppt_1_voltage(voltage_data);
        set_mppt_1_pwm(pwm_data);
        set_mppt_1_status(status_data);
        break;
      case 2:
        set_mppt_2_current(current_data);
        set_mppt_2_voltage(voltage_data);
        set_mppt_2_pwm(pwm_data);
        set_mppt_2_status(status_data);
        break;
      case 3:
        set_mppt_3_current(current_data);
        set_mppt_3_voltage(voltage_data);
        set_mppt_3_pwm(pwm_data);
        set_mppt_3_status(status_data);
        break;
      case 4:
        set_mppt_4_current(current_data);
        set_mppt_4_voltage(voltage_data);
        set_mppt_4_pwm(pwm_data);
        set_mppt_4_status(status_data);
        break;
      case 5:
        set_mppt_5_current(current_data);
        set_mppt_5_voltage(voltage_data);
        set_mppt_5_pwm(pwm_data);
        set_mppt_5_status(status_data);
        break;
      case 6:
        set_mppt_6_current(current_data);
        set_mppt_6_voltage(voltage_data);
        set_mppt_6_pwm(pwm_data);
        set_mppt_6_status(status_data);
        break;
    }
  }

  return STATUS_CODE_OK;
}
