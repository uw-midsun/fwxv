#pragma once 
#include "fsm.h"

void prv_get_precharge_input(Fsm *fsm, void *context);
void prv_get_precharge_output(void *context);

void prv_do_precharge_input(Fsm *fsm, void *context);
void prv_do_precharge_output(void *context);

void prv_transmit_input(Fsm *fsm, void *context);
void prv_transmit_output(void *context);