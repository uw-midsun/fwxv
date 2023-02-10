#pragma once

#include "power_select_setters.h"
// TODO: should use getter, maybe add getters for tx_struct as well
// or maybe have a single can data struct with everything
#define POWER_SELECT_STATUS (uint8_t)(g_tx_struct.power_select_status >> 56)
#define POWER_SELECT_FAULT (uint8_t)(g_tx_struct.power_select_status >> 48)
