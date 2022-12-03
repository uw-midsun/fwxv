#pragma once

#include "delay.h"

typedef struct MotorData {
  // updated every 200ms
  uint8_t velocity;  // (m/s)
  int16_t rpm;
  uint16_t bus_voltage_v;
  uint16_t bus_current_a;
  uint8_t mc_limit_bitset;
  uint8_t mc_error_bitset;
  // updated every second
  uint32_t motor_temp_c;
  uint32_t heatsink_temp_c;
  uint32_t dsp_temp_c;
} MotorData;

MotorData left_motor;
MotorData right_motor;
