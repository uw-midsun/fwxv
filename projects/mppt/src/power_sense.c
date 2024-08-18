#include "power_sense.h"

MPPTData *mppt;

void power_sense_init(MPPTData *init_mppt) {
  mppt = init_mppt;
}

void voltage_sense() {}

void current_sense() {}

void calculate_power() {}

void set_voltage(uint32_t voltage) {
  mppt->voltage = voltage;
}

void set_current(uint32_t current) {
  mppt->current = current;
}

void set_power(uint32_t power) {
  mppt->power = power;
}
