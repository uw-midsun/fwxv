#pragma once

#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "status.h"

StatusCode fault_bps_init(BmsStorage *storage);

StatusCode fault_bps_set(uint8_t fault_bitmask);

StatusCode fault_bps_clear(uint8_t fault_bitmask);

uint8_t fault_bps_get(void);
