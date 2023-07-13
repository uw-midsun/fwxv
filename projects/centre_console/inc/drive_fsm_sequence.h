#pragma once

#include "drive_fsm.h"
#include "fsm.h"
#include "log.h"

#define BEGIN_PRECHARGE_SIGNAL 2
#define PRECHARGE_STATE_COMPLETE 2
#define NUMBER_OF_CYCLES_TO_WAIT 10

void prv_do_precharge_input(Fsm *fsm, void *context);
void prv_do_precharge_output(void *context);

void prv_transmit_input(Fsm *fsm, void *context);
void prv_transmit_output(void *context);
