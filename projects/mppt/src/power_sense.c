#include "power_sense.h"

MPPTData *mppt;

void power_sense_init(MPPTData *init_mppt) {
  mppt = init_mppt;
}

void voltage_sense() {}

void current_sense() {}

void calculate_prev_power() {
  // For testing
  mppt->prev_power = mppt->prev_voltage * mppt->prev_current;
}

void calculate_power() {
  mppt->power = mppt->voltage * mppt->current;
  // Maybe add filtering? I expect this to run at fastest possible frequency so steady-state noise
  // is expected
}
