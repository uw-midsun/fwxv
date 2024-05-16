#include "gpio.h"
#include "log.h"
#include "pwm.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

// TIM 1
static const GpioAddress pwm_tim_1[] = {
  { .port = GPIO_PORT_A, .pin = 8 },
  { .port = GPIO_PORT_A, .pin = 9 },
  { .port = GPIO_PORT_A, .pin = 10 },
  { .port = GPIO_PORT_A, .pin = 11 },
};

// TIM 1N
static const GpioAddress pwm_tim_1n[] = {
  { .port = GPIO_PORT_B, .pin = 13 },
  { .port = GPIO_PORT_B, .pin = 14 },
  { .port = GPIO_PORT_B, .pin = 15 },
};

// TIM 3
static const GpioAddress pwm_tim_3[] = {
  { .port = GPIO_PORT_A, .pin = 6 },
  { .port = GPIO_PORT_A, .pin = 7 },
  { .port = GPIO_PORT_B, .pin = 0 },
  { .port = GPIO_PORT_B, .pin = 1 },
};

// TIM 4
static const GpioAddress pwm_tim_4[] = {
  { .port = GPIO_PORT_B, .pin = 6 },
  { .port = GPIO_PORT_B, .pin = 7 },
  { .port = GPIO_PORT_B, .pin = 8 },
  { .port = GPIO_PORT_A, .pin = 9 },
};

int main(void) {
  gpio_init();
  log_init();

  // Initialize PWM pins for TIM1
  for (uint8_t i = 0; i < SIZEOF_ARRAY(pwm_tim_1); i++) {
    gpio_init_pin(&pwm_tim_1[i], GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  }
  // Initialize PWM pins for TIM1_N
  for (uint8_t i = 0; i < SIZEOF_ARRAY(pwm_tim_1n); i++) {
    gpio_init_pin(&pwm_tim_1n[i], GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  }
  pwm_init(PWM_TIMER_1, 250);
  pwm_set_dc(PWM_TIMER_1, 50);

  // Initialize PWM pins for TIM3
  for (uint8_t i = 0; i < SIZEOF_ARRAY(pwm_tim_3); i++) {
    gpio_init_pin(&pwm_tim_3[i], GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  }
  pwm_init(PWM_TIMER_3, 250);
  pwm_set_dc(PWM_TIMER_3, 50);

  // Initialize PWM pins for TIM4
  for (uint8_t i = 0; i < SIZEOF_ARRAY(pwm_tim_4); i++) {
    gpio_init_pin(&pwm_tim_4[i], GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  }
  pwm_init(PWM_TIMER_4, 250);
  pwm_set_dc(PWM_TIMER_4, 50);

  while (1) {
    // Main loop
  }
}
