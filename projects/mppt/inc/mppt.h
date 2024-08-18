#pragma once

#include <stdint.h>

/**
 * (ARYAN'S UNDERSTANDING. WILL BE IMPROVED)
 * MPPTs (Maximum Power Point Tracking) search for the optimal voltage and current to produce the
 * max power By controlling the switching frequency of the buck-boost, we can control the maximum
 * charge current exiting the array The array's voltage stays relatively constant throughout use,
 * only the current changes. A low impedence can be mocked by a high duty-cycle, whereas a high
 * impedence is mocked by a low duty-cycle
 */

#define MAX_DUTY_CYCLE 1000
#define MIN_DUTY_CYCLE 0
#define DUTY_CYCLE_STEP (MAX_DUTY_CYCLE - MIN_DUTY_CYCLE) / 200  // 5% Step increments

typedef enum {
  MPPT_PERTURB_OBSERVE,
  MPPT_CONSTANT_VOLTAGE,
  MPPT_INCREMENTAL_CONDUCTANCE,
  MPPT_STOPPED,
  MPPT_UNITIALIZED,
} MPPTAlgorithm;

typedef struct {
  MPPTAlgorithm algorithm;

  uint32_t voltage;
  uint32_t current;
  uint32_t power;

  uint32_t prev_voltage;
  uint32_t prev_current;
  uint32_t prev_power;
  uint16_t pwm_dc;
} MPPTData;

void mppt_init(MPPTData *init_mppt);
void mppt_run(void);
void mppt_stop(void);  // In fault we stop the MPPT

void mppt_set_algorithm(MPPTAlgorithm algo);
void mppt_set_pwm(uint16_t pwm_dc);
