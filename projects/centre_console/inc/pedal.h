#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "calib.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_calib.h"
#include "status.h"

#define BRAKE_LIMIT_SWITCH \
  { .port = GPIO_PORT_B, .pin = 13 }

#define ADC_HALL_SENSOR \
  { .port = GPIO_PORT_A, .pin = 0 }

void pedal_run();

StatusCode pedal_init(PedalCalibBlob *calib_blob);
