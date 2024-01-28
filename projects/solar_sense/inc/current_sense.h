#pragma once
#include <stdint.h>

#include "gpio.h"
#include "status.h"


StatusCode current_sense_main_cycle();

StatusCode current_sense_relay_set(GpioState state);
