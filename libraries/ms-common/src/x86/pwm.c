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
StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, uint8_t channel,
                         bool n_channel_en) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (pwm[timer].period == 0) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "Pwm must be initialized.");
  } else if (pulse_width_us > pwm[timer].period) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Pulse width must be leq period.");
  }
  // Store pulse width as a duty cycle
  pwm[timer].duty = (pulse_width_us * 100) / pwm[timer].period;
  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, uint8_t channel, bool n_channel_en) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (dc > 100) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle must be leq 100%.");
  }
  pwm[timer].duty = dc;

  return STATUS_CODE_OK;
}

uint16_t pwm_get_dc(PwmTimer timer) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  }
  return pwm[timer].duty;
}
