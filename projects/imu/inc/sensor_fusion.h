#pragma once

#include "status.h"

#define DEGREE_TO_RADS 0.0174532

typedef struct {
  float beta_def;   // Defines the Mahony filter gain. Balance between gyroscope and mag/accel data
  float two_kp;     // Porportional gain to determine how strongly the algo corrects errors from perdicted
                    // data/measured data
  float two_ki;     // Integral gain helps correct long term biases in measurement
  float q0 = 1.0f;  // Quaternion component to represent the orientation (X)
  float q1 = 0.0f;  // Quaternion component to represent the orientation (Y)
  float q2 = 0.0f;  // Quaternion component to represent the orientation (Z)
  float q3 = 0.0f;  // Quaternion component to represent the orientation (W)
  float integral_FBx = 0.0f;  // Stores integral term of X axis. Used for error build up and long-term drift
  float integral_FBy = 0.0f;  // Stores integral term of X axis. Used for error build up and long-term drift
  float integral_FBz = 0.0f;  // Stores integral term of X axis. Used for error build up and long-term drift
  uint32_t last_time;
  bool angles_computed = 0;
} MahonyStorage;

void mahony_update(uint16_t gx, uint16_t gy, uint16_t gz, uint16_t ax, uint16_t ay, uint16_t az,
                   uint16_t mx, uint16_t my, uint16_t mz, uint32_t delta_time);

void mahony_update(uint16_t gx, uint16_t gy, uint16_t gz, uint16_t ax, uint16_t ay, uint16_t az,
                   uint32_t delta_time);
