#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "calib.h"
#include "cc_hw_defs.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_calib.h"
#include "status.h"

// Runs a pedal cycle to update throttle and brake values
void pedal_run();
uint16_t pedal_map();

// Initializes pedal peripherals
// Must be called before adc_init
StatusCode pedal_init(PedalCalibBlob *calib_blob);
