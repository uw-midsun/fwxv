#pragma once

#include <stdint.h>

#include "spi_mcu.h"
#include "status.h"

// Turns on MPPT
StatusCode mppt_init(const uint8_t mppt_idx);

// Reads current from MPPT
StatusCode mppt_read_current(const uint8_t mppt_idx, uint16_t *current_data);

// Reads vin from MPPT
StatusCode mppt_read_voltage(const uint8_t mppt_idx, uint16_t *voltage_data);

// Reads pwm from MPPT
StatusCode mppt_read_pwm(const uint8_t mppt_idx, uint16_t *pwm_data);

// Reads status from MPPT
StatusCode mppt_read_status(const uint8_t mppt_idx, uint16_t *status_data);
