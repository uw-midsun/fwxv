#pragma once

#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "can.h"
#include "delay.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "task.h"

#define BMS_CLOSE_RELAYS_DELAY 300
typedef enum RelayType { NO_RELAYS = 0, POS_RELAY, NEG_RELAY, SOLAR_RELAY } RelayType;

void bms_relay_fault(void);

StatusCode init_bms_relays(void);
