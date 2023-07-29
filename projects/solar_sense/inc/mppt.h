#pragma once
#include <gpio.h>
#include <i2c.h>
#include <log.h>
#include <spi.h>

void mppt_init();
StatusCode mppt_shut(SpiPort port, GpioAddress pin);
StatusCode mppt_turn_on(SpiPort port, GpioAddress pin);
StatusCode mppt_read_current(SpiPort port, uint16_t *current, GpioAddress pin);
// StatusCode mppt_read_voltage_in(SpiPort port, uint16_t* vin, GpioAddress pin);
StatusCode mppt_read_pwm(SpiPort port, uint16_t *pwm, GpioAddress pin);
StatusCode mppt_read_status(SpiPort port, uint8_t *status, GpioAddress pin);
