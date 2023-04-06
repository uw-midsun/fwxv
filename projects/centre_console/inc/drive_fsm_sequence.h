#pragma once 

#include "drive_fsm.h"
#include "fsm.h"
#include "log.h"


void prv_do_precharge_input(Fsm *fsm, void *context);
void prv_do_precharge_output(void *context);

void prv_transmit_input(Fsm *fsm, void *context);
void prv_transmit_output(void *context);