#pragma once

#include "adc.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "gpio.h"
#include "log.h"
#include "pwm.h"

#define BMS_FAN_PERIOD 0.04              // Period in ms. Frequency = 25,000 Hz
#define BMS_FAN_TEMP_UPPER_THRESHOLD 55  // Threshold before fan is full strength
#define BMS_FAN_TEMP_LOWER_THRESHOLD 40  // Threshold before fan is full strength

void bms_run_fan(void);
void bms_fan_init(void);
