#include "fan.h"

BmsStorage *s_storage;

const GpioAddress bms_fan_sense1 = { .port = GPIO_PORT_B, .pin = 1 };
const GpioAddress bms_fan_sense2 = { .port = GPIO_PORT_B, .pin = 0 };
const GpioAddress bms_fan_pwm = { .port = GPIO_PORT_A, .pin = 8 };

static void prv_bms_fan_sense(void) {
  LOG_DEBUG("RUN FAN SENSE\n");
  TickType_t tick = xTaskGetTickCount() + pdMS_TO_TICKS(5);
  GpioState state = 1;

  while (xTaskGetTickCount() < tick && state) {
    // LOG_DEBUG("STATE FANSENSE_1: %d\n", state);
    gpio_get_state(&bms_fan_sense1, &state);
  }
  set_battery_info_fan1(!state);

  tick = xTaskGetTickCount() + pdMS_TO_TICKS(5);

  while (xTaskGetTickCount() < tick && state) {
    // LOG_DEBUG("STATE FANSENSE_2: %d\n", state);
    gpio_get_state(&bms_fan_sense2, &state);
  }
  set_battery_info_fan2(!state);
  // LOG_DEBUG("FAN1: %d\n", g_tx_struct.battery_status_fan1);
  // LOG_DEBUG("FAN2: %d\n", g_tx_struct.battery_status_fan2);
}

static inline uint8_t prv_calc_fan_speed(uint16_t max_temp) {
  // Scale percent based on temp range, starting at 50% if fan is set on
  uint8_t speed_perc = 100 * ((float)max_temp - BMS_FAN_TEMP_LOWER_THRESHOLD) /
                       (float)(BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD);
  return speed_perc / 2 + 50 > 100 ? 100 : speed_perc / 2 + 50;
}

void bms_run_fan(void) {
  if (s_storage->ltc_afe_storage.max_temp >= BMS_FAN_TEMP_UPPER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_1, 100, 1, false);
  } else if (s_storage->ltc_afe_storage.max_temp <= BMS_FAN_TEMP_LOWER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_1, 0, 1, false);
  } else {
    pwm_set_dc(PWM_TIMER_1, prv_calc_fan_speed(s_storage->ltc_afe_storage.max_temp), 1, false);
  }
  prv_bms_fan_sense();
}

void bms_fan_init(BmsStorage *bms_storage) {
  s_storage = bms_storage;
  gpio_init_pin(&bms_fan_pwm, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_LOW);
  pwm_init(PWM_TIMER_1, BMS_FAN_PERIOD);
  pwm_set_pulse(PWM_TIMER_1, BMS_FAN_PERIOD, 1, false);
  pwm_set_dc(PWM_TIMER_1, 0, 1, false);

  gpio_init_pin(&bms_fan_sense1, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&bms_fan_sense2, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
}
