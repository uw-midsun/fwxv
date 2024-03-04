#include "fan.h"

const GpioAddress bms_fan_sense1 = { .port = GPIO_PORT_B, .pin = 1 };
const GpioAddress bms_fan_sense2 = { .port = GPIO_PORT_B, .pin = 0 };

void bms_run_fan(void) {
  // if (temperature < 20) {
  //     pwm_set_dc(PWM_TIMER_3, (temperature * 100) / BMS_FAN_TEMP_THRESHOLD);
  // } else {
  pwm_set_dc(PWM_TIMER_3, 100);
  LOG_DEBUG("RUNNING FAN CYCLE\n");
  // }
}

// void bms_fan_rpm(void) {
//     uint16_t start_time, end_time, period;
//     GpioState state;

//     if (gpio_get_state(&bms_fan_sense1, &state) == GPIO_STATE_LOW) {
//         falling_edge = hw_timer_get_tick();
//     } else {
//         rising_edge = hw_timer_get_tick();
//     }
// }

void bms_fan_init(void) {
  pwm_init(PWM_TIMER_3, 40);
  pwm_set_pulse(PWM_TIMER_3, 40);
  pwm_set_dc(PWM_TIMER_3, 0);
  // TODO: Add in RPM detecting
  gpio_init_pin(&bms_fan_sense1, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&bms_fan_sense2, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
}
