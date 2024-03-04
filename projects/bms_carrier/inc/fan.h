#pragma once

#include "gpio.h"
#include "log.h"
#include "pwm.h"

#define BMS_FAN_PERIOD 0.04        // Period in ms. Frequency = 25,000 Hz
#define BMS_FAN_TEMP_THRESHOLD 20  // Threshold before fan is full strength

void bms_run_fan(void);

void bms_fan_init(void);
