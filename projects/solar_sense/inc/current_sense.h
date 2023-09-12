#pragma once
#include <gpio.h>
#include <i2c.h>
#include <log.h>

#include "can.h"
#include "can_board_ids.h"

StatusCode read_relay_status(uint8_t *relay_status);
StatusCode read_voltage(uint16_t *voltage);
StatusCode read_current_sense(uint16_t *current);
StatusCode relay_fault();
StatusCode solar_sense_relay_close();
StatusCode solar_sense_relay_open();
