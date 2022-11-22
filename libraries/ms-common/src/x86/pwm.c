#include "pwm.h"

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

typedef struct PwmInfo {
  uint16_t period;
  uint16_t duty;
} PwmInfo;

PwmInfo pwm[NUM_PWM_TIMERS];

StatusCode pwm_init(PwmTimer timer, uint16_t period_ms) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (period_ms == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Period must be greater than 0");
  }

  pwm[timer].period = period_ms;
  return STATUS_CODE_OK;
}

uint16_t pwm_get_period(PwmTimer timer) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  }
  return pwm[timer].period;
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_ms) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (pwm[timer].period == 0) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "Pwm must be initialized.");
  } else if (pulse_width_ms > pwm[timer].period) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Pulse width must be leq period.");
  }

  pwm[timer].duty = (pulse_width_ms / pwm[timer].period) * 100;
  
  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (dc > 100) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle must be leq 100%.");
  }

  uint16_t pulse_width = 0;
  if (dc != 0) {
    pulse_width = ((pwm[timer].period + 1) * dc) / 100 - 1;
    if (pulse_width == 0) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle is not valid for given period.");
    }
  }

  return pwm_set_pulse(timer, pulse_width);
}

uint16_t pwm_get_dc(PwmTimer timer) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  }
  return pwm[timer].duty;
}

