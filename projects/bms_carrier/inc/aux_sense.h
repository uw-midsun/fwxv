#pragma once

#include "adc.h"
#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "gpio.h"

#define R2_OHMS 10000
#define R1_OHMS 47000

StatusCode aux_sense_init(BmsStorage *storage);

StatusCode aux_sense_run(void);
