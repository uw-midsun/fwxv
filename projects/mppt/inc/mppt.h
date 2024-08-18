#pragma once

#include <stdint.h>

/**
 * MPPTs (Maximum Power Point Tracking) constantly adjust the operating point of solar panels
 * to ensure they are operating at their maximum power point (MPP), where the power output
 * (voltage * current) is maximized.
 *
 * By controlling the duty cycle of the buck-boost converter, we control the voltage and
 * current characteristics at the output of the converter, adjusting the operating point
 * of the solar array.
 *
 * The duty cycle effectively controls the load impedance seen by the solar array:
 * A high duty cycle corresponds to lower impedance, allowing more current to flow.
 * A low duty cycle corresponds to higher impedance, reducing current flow.
 *
 * Think about Thevenin impedence loads (Maximum power output)
 *
 * A disadvantage of always having to alter the duty-cycle to determine the MPP is a loss of power
 * when an external change occurs (Slow resposne-time and inaccurate steady-state value)
 *
 */

#define MAX_DUTY_CYCLE 1000
#define MIN_DUTY_CYCLE 0
#define DUTY_CYCLE_STEP (MAX_DUTY_CYCLE - MIN_DUTY_CYCLE) / 200  // 0.5% Step increments

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

void mppt_stop(void);  // In fault we stop the MPPT

/**
 *
 *
 *
 *
 *
 *
 */
void mppt_run_preturb();

/**
 * Incremental conductance detects the slope of the P-V curve, and searches for the peak of the P-V
 * curve. If dP/dV is positive, the nearest maximum is at a higher voltage. If dP/dV is negative the
 * nearest maximum is at a lower voltage. When dP/dV is 0, the maximum power has been reached dP/dV
 * = d(I x V)/dV = I dV/dV + V dI/dV = 0 dI/dV = -I/V, where dI/dV is the incremental conductance
 * and I/V is the instantaneous conductance
 *
 * https://pcmp.springeropen.com/articles/10.1186/s41601-020-00161-z
 * We will use a modified algorithim with ~20-30% faster resposne time to changes in load/irradiance
 */
void mppt_run_incremental_conductance();

void mppt_run(void);

void mppt_set_algorithm(MPPTAlgorithm algo);
void mppt_set_pwm(uint16_t pwm_dc);
