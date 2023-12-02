#ifndef BPS_INDICATOR_H
#define BPS_INDICATOR_H

#include <stdio.h>
#include "soft_timer.h" 
#include "outputs.h"

// callback function to periodically toggle the state of the bps fault indicator
static void prv_callback();

// functions to start/stop bps fault indicator
static void start_bps_fault_indicator();
static void stop_bps_fault_indicator();

#endif 