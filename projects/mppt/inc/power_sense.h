#pragma once

#include <stdint.h>

#include "mppt.h"

void power_sense_init(MPPTData *init_mppt);
void voltage_sense();
void current_sense();
void calculate_power();

// Test functions
void set_voltage(uint32_t voltage);
void set_current(uint32_t current);
void set_power(uint32_t power);
