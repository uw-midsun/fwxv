#pragma once
#include <stdint.h>

#include "gpio.h"
#include "status.h"

// LTC2451 ADC Possible conversion speeds
uint8_t set_30Hz_speed[] = { 0x80 };
uint8_t set_60Hz_speed[] = { 0x00 };

StatusCode current_sense_init(uint8_t *conversion_speed);

StatusCode current_sense_main_cycle(uint16_t *voltage_measured);

StatusCode current_sense_relay_set(GpioState state);
