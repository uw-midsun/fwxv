#pragma once

#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "can.h"
#include "current_sense.h"
#include "delay.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "ltc_afe.h"
#include "ltc_afe_impl.h"
#include "status.h"
#include "task.h"

#define NUM_RELAY_STATES 3
#define FUEL_GAUGE_CYCLE_TIME_MS 100

DECLARE_FSM(bms_relays);
typedef enum RelaysStateId { RELAYS_OPEN = 0, RELAYS_CLOSED, RELAYS_FAULT } RelaysStateId;

StatusCode init_bms_relays(void);
