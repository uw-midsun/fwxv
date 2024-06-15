#include "pwm.h"

#include <stdint.h>

#include "gpio.h"
#include "pwm_mcu.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

#define NUM_CHANNELS 4
typedef enum APBClk {
  APB1 = 0,
  APB2,
  NUM_APB_CLK,
} APBClk;

static uint16_t s_period_us[NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = 0,   //
  [PWM_TIMER_3] = 0,   //
  [PWM_TIMER_4] = 0,   // This timer is typically used by I2C
  [PWM_TIMER_14] = 0,  //
  [PWM_TIMER_15] = 0,  //
  [PWM_TIMER_16] = 0,  //
  [PWM_TIMER_17] = 0,  //
};

static TIM_TypeDef *s_timer_def[NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = TIM1,    //
  [PWM_TIMER_3] = TIM3,    //
  [PWM_TIMER_4] = TIM4,    // Typically used for I2C
  [PWM_TIMER_14] = TIM14,  //
  [PWM_TIMER_15] = TIM15,  //
  [PWM_TIMER_16] = TIM16,  //
  [PWM_TIMER_17] = TIM17,  //
};

static APBClk prv_enable_periph_clock(PwmTimer timer) {
  switch (timer) {
    case PWM_TIMER_1:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
      return APB2;
    case PWM_TIMER_3:
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
      return APB1;
    case PWM_TIMER_4:
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
      return APB1;
    case PWM_TIMER_14:
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
      return APB1;
    case PWM_TIMER_15:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
      return APB2;
    case PWM_TIMER_16:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
      return APB2;
    case PWM_TIMER_17:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
      return APB2;
    default:
      return NUM_APB_CLK;
  }
}

StatusCode pwm_init(PwmTimer timer, uint16_t period_us) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (period_us == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Period must be greater than 0");
  }

  APBClk clk = prv_enable_periph_clock(timer);

  s_period_us[timer] = period_us;

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = period_us,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  if (clk == APB1) {
    tim_init.TIM_Prescaler = (clocks.PCLK1_Frequency / 1000000) - 1;
  } else {
    tim_init.TIM_Prescaler = (clocks.PCLK2_Frequency / 1000000) - 1;
  }

  TIM_TimeBaseInit(s_timer_def[timer], &tim_init);
  TIM_Cmd(s_timer_def[timer], ENABLE);
  TIM_CtrlPWMOutputs(s_timer_def[timer], ENABLE);

  return STATUS_CODE_OK;
}

uint16_t pwm_get_period(PwmTimer timer) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  }
  return s_period_us[timer];
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us, uint8_t channel) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (s_period_us[timer] == 0) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "Pwm must be initialized.");
  } else if (pulse_width_us > s_period_us[timer]) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Pulse width must be leq period.");
  }

  TIM_OCInitTypeDef oc_init = {
    .TIM_OCMode = TIM_OCMode_PWM1,  // Set on compare match.
    .TIM_Pulse = pulse_width_us,
    .TIM_OutputState = TIM_OutputState_Enable,
    .TIM_OCPolarity = TIM_OCPolarity_High,
    .TIM_OutputNState = TIM_OutputNState_Disable,
    .TIM_OCNPolarity = TIM_OCNPolarity_High,
  };

  // Enable PWM on all channels.
  switch (channel) {
    case 1:
      TIM_OC1Init(s_timer_def[timer], &oc_init);
      TIM_OC1PreloadConfig(s_timer_def[timer], TIM_OCPreload_Enable);
      break;
    case 2:
      TIM_OC2Init(s_timer_def[timer], &oc_init);
      TIM_OC2PreloadConfig(s_timer_def[timer], TIM_OCPreload_Enable);
      break;
    case 3:
      TIM_OC3Init(s_timer_def[timer], &oc_init);
      TIM_OC3PreloadConfig(s_timer_def[timer], TIM_OCPreload_Enable);
      break;
    case 4:
      TIM_OC4Init(s_timer_def[timer], &oc_init);
      TIM_OC4PreloadConfig(s_timer_def[timer], TIM_OCPreload_Enable);
      break;
    default:
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc, uint8_t channel) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (dc > 100) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle must be leq 100%.");
  }

  uint16_t pulse_width = 0;
  if (dc != 0) {
    pulse_width = ((s_period_us[timer] + 1) * dc) / 100 - 1;
    if (pulse_width == 0) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle is not valid for given period");
    }
  }

  return pwm_set_pulse(timer, pulse_width, channel);
}

uint16_t pwm_get_dc(PwmTimer timer) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
