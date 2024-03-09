#include "fan.h"

const GpioAddress bms_fan_sense1 = { .port = GPIO_PORT_B, .pin = 1 };
const GpioAddress bms_fan_sense2 = { .port = GPIO_PORT_B, .pin = 0 };

static void prv_bms_fan_sense(void) {
  LOG_DEBUG("RUN FAN SENSE\n");
  TickType_t tick = xTaskGetTickCount() + pdMS_TO_TICKS(5);
  GpioState state = 0;

  while (!state && xTaskGetTickCount() < tick) {
    gpio_get_state(&bms_fan_sense1, &state);
    // LOG_DEBUG("STATE FANSENSE_1: %d\n", state);
  }

  tick = xTaskGetTickCount() + pdMS_TO_TICKS(5);

  while (!state && xTaskGetTickCount() < tick) {
    gpio_get_state(&bms_fan_sense1, &state);
    // LOG_DEBUG("STATE FANSENSE_2: %d\n", state);
  }
  set_battery_status_fan(state);
  LOG_DEBUG("%d\n", g_tx_struct.battery_status_fan);
}

void bms_run_fan(void) {
  // g_tx_struct.battery_vt_temperature = 50;
  if (g_tx_struct.battery_vt_temperature >= BMS_FAN_TEMP_UPPER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_3, 100);
  } else if (g_tx_struct.battery_vt_temperature <= BMS_FAN_TEMP_LOWER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_3, 0);
  } else {
    pwm_set_dc(PWM_TIMER_3,
               (100 * (g_tx_struct.battery_vt_temperature - BMS_FAN_TEMP_LOWER_THRESHOLD)) /
                   (BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD));
  }
  prv_bms_fan_sense();
}

void bms_fan_init(void) {
  pwm_init(PWM_TIMER_3, 40);
  pwm_set_pulse(PWM_TIMER_3, 40);
  pwm_set_dc(PWM_TIMER_3, 0);

  gpio_init_pin(&bms_fan_sense1, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&bms_fan_sense2, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
}
