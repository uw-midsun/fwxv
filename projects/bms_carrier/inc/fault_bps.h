#pragma once

#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "relays.h"
#include "status.h"

typedef struct BpsStorage {
  uint16_t fault_bitset;
} BpsStorage;

StatusCode fault_bps_init(BpsStorage *storage);

StatusCode fault_bps_set(uint8_t fault_bitmask);

StatusCode fault_bps_clear(uint8_t fault_bitmask);
