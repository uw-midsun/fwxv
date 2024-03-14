#pragma once

#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "can.h"
#include "delay.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "status.h"
#include "task.h"

#define NUM_RELAY_STATES 3
#define KILLSWITCH_IT 1

DECLARE_FSM(bms_relays);
typedef enum RelaysStateId { RELAYS_OPEN = 0, RELAYS_CLOSED, RELAYS_FAULT } RelaysStateId;

StatusCode init_bms_relays(BmsStorage *bms_storage);

StatusCode relays_fault();
