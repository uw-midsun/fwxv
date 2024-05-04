#include "gpio.h"
#include "log.h"
#include "pwm.h"

GpioAddress pin = { .port = GPIO_PORT_B, .pin = 15 };
int main(void) {
  gpio_init();
  log_init();
  gpio_init_pin(&pin, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_HIGH);
  pwm_init(PWM_TIMER_1, 250);
  pwm_set_dc(PWM_TIMER_1, 50);
  while (1) {
  }
}
