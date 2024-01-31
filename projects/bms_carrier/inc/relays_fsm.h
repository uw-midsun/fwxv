#pragma once

#include "can.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"
#include "gpio.h"
#include "status.h"
#include "task.h"
#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"

#define NUM_RELAY_STATES 3

void close_relays();
void open_relays();

DECLARE_FSM(relays);
typedef enum RelaysStateId { RELAYS_OPEN = 0, RELAYS_CLOSED, RELAYS_FAULT } RelaysStateId;

StatusCode init_relays(void);
