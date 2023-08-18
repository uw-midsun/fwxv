#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bts_load_switch.h"
#include "gpio.h"
#include "i2c.h"
#include "outputs.h"
#include "pca9555_gpio_expander.h"
#include "pin_defs.h"

// initializes sense pin and mux sel pins
StatusCode pd_sense_init(void);

// reads adc value of all pins in output group and stores the result in reading_out attr
StatusCode pd_sense_output_group(OutputGroup group);
