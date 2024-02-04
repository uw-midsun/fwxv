#pragma once

#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "can.h"
#include "current_sense.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "status.h"
#include "task.h"

#define NUM_RELAY_STATES 3

void close_relays();
void open_relays();

DECLARE_FSM(relays);
typedef enum RelaysStateId { RELAYS_OPEN = 0, RELAYS_CLOSED, RELAYS_FAULT } RelaysStateId;

StatusCode init_relays(void);
