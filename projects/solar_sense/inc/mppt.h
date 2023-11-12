#pragma once

#include "spi.h"

// Turns on all MPPTs
StatusCode mppt_init(const SpiSettings *settings, const SpiPort port);

// Top level function to read data from all MPPTs and send it to telemetry
StatusCode read_mppts();

// Reads current from MPPT
StatusCode mppt_read_current(uint16_t *current_data);

// Reads vin from MPPT
StatusCode mppt_read_voltage(uint16_t *voltage_data);

// Reads pwm from MPPT
StatusCode mppt_read_pwm(uint16_t *pwm_data);

// Reads status from MPPT
StatusCode mppt_read_status(uint16_t *status_data);
