#pragma once

#include <stdint.h>

#include "spi_mcu.h"
#include "status.h"

// Turns on MPPT
StatusCode mppt_init(SpiPort spi_port, uint8_t gpio_port);

// Reads current from MPPT
StatusCode mppt_read_current(SpiPort spi_port, uint16_t *current_data);

// Reads vin from MPPT
StatusCode mppt_read_voltage(SpiPort spi_port, uint16_t *voltage_data);

// Reads pwm from MPPT
StatusCode mppt_read_pwm(SpiPort spi_port, uint16_t *pwm_data);

// Reads status from MPPT
StatusCode mppt_read_status(SpiPort spi_port, uint16_t *status_data);
