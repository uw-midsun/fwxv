#pragma once

#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "can.h"
#include "delay.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "task.h"

#define NUM_RELAY_STATES 3
#define BMS_CLOSE_RELAYS_DELAY 200

DECLARE_FSM(bms_relays);
typedef enum RelaysStateId { RELAYS_OPEN = 0, RELAYS_CLOSED, RELAYS_FAULT } RelaysStateId;
typedef enum RelayType { NO_RELAYS = 0, POS_RELAY, NEG_RELAY, SOLAR_RELAY } RelayType;

StatusCode init_bms_relays(BmsStorage *bms_storage);
