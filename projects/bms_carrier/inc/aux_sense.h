#pragma once

#include "adc.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "gpio.h"

#define R2_OHMS 10000
#define R1_OHMS 47000

typedef struct AuxStorage {
  uint16_t aux_battery_voltage;
} AuxStorage;

StatusCode aux_sense_init(AuxStorage *aux_storage);

StatusCode aux_sense_run(void);
