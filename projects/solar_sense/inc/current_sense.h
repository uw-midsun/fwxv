#pragma once
#include <stdint.h>

#include "gpio.h"
#include "status.h"

// LTC2451 Possible conversion speeds
uint8_t set_30Hz_speed[] = { 0x80 };
uint8_t set_60Hz_speed[] = { 0x00 };

StatusCode ltc2451_adc_init();

StatusCode drv120_relay_init();

StatusCode ltc2451_adc_read_converted(uint16_t *voltage_measured);

StatusCode drv120_relay_set(GpioState state);
