#pragma once

#include <stdint.h>

#include "mppt.h"

#define POWER_WEIGHT 0.3f

void power_sense_init(MPPTData *init_mppt);
void voltage_sense();
void current_sense();
void calculate_prev_power();
void calculate_power();
