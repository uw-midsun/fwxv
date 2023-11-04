#pragma once

#include <stdint.h>

#include "spi_mcu.h"
#include "status.h"

// Turns on MPPT
StatusCode mppt_init(uint8_t mppt_num);

// Reads current from MPPT
StatusCode mppt_read_current(uint8_t mppt_num);

// Reads vin from MPPT
StatusCode mppt_read_voltage(uint8_t mppt_num);

// Reads pwm from MPPT
StatusCode mppt_read_pwm(uint8_t mppt_num);

// Reads status from MPPT
StatusCode mppt_read_status(uint8_t mppt_num);
