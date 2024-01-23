#pragma once

#include "can.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"
#include "status.h"
#include "task.h"

#define NUM_RELAY_STATES 3

DECLARE_FSM(relays);
typedef enum RelaysStateId { RELAYS_OPEN = 0, RELAYS_CLOSED, RELAYS_FAULT } RelaysStateId;

StatusCode init_relays(void);
