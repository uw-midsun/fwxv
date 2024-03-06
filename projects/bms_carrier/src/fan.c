#include "fan.h"

const GpioAddress bms_fan_sense1 = { .port = GPIO_PORT_B, .pin = 1 };
const GpioAddress bms_fan_sense2 = { .port = GPIO_PORT_B, .pin = 0 };

void bms_run_fan(void) {
  g_tx_struct.battery_vt_temperature = 50;
  if (g_tx_struct.battery_vt_temperature >= BMS_FAN_TEMP_UPPER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_3, 100);
  } else if (g_tx_struct.battery_vt_temperature <= BMS_FAN_TEMP_LOWER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_3, 0);
  } else {
    pwm_set_dc(PWM_TIMER_3,
               (100 * (g_tx_struct.battery_vt_temperature - BMS_FAN_TEMP_LOWER_THRESHOLD)) /
                   (BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD));
  }
}

void bms_fan_sense(void) {
  TickType_t tick = xTaskGetTickCount() + pdMS_TO_TICKS(4);
  GpioState state;
  LOG_DEBUG("STATE FANSENSE_1: %d\n", gpio_get_state(&bms_fan_sense1, &state));
  while(!gpio_get_state(&bms_fan_sense1, &state) && xTaskGetTickCount() < tick) {
      LOG_DEBUG("STATE 1 %d\n", state);
  };
  LOG_DEBUG("STATE FANSENSE_2: %d\n", gpio_get_state(&bms_fan_sense2, &state));
  while(!gpio_get_state(&bms_fan_sense2, &state) && xTaskGetTickCount() < tick) {
    LOG_DEBUG("STATE 2 %d\n", state);
  }
  LOG_DEBUG("RUN FAN SENSE\n");
}

void bms_fan_init(void) {
  pwm_init(PWM_TIMER_3, 40);
  pwm_set_pulse(PWM_TIMER_3, 40);
  pwm_set_dc(PWM_TIMER_3, 0);
  // TODO: Add in RPM detecting
  gpio_init_pin(&bms_fan_sense1, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&bms_fan_sense2, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
}
