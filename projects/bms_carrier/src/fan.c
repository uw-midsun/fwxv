#include "fan.h"

BmsStorage *s_storage;

const GpioAddress bms_fan_sense1 = { .port = GPIO_PORT_B, .pin = 1 };
const GpioAddress bms_fan_sense2 = { .port = GPIO_PORT_B, .pin = 0 };

// The fan sense pin is high when it is off. Low when it is on
// Todo: Maybe check for interrupt?
static void prv_bms_fan_sense(void) {
  LOG_DEBUG("RUN FAN SENSE\n");
  TickType_t tick = xTaskGetTickCount() + pdMS_TO_TICKS(5);
  GpioState state = 1;

  while (xTaskGetTickCount() < tick && state) {
    // LOG_DEBUG("STATE FANSENSE_1: %d\n", state);
    gpio_get_state(&bms_fan_sense1, &state);
  }
  set_battery_status_fan1(!state);

  tick = xTaskGetTickCount() + pdMS_TO_TICKS(5);

  while (xTaskGetTickCount() < tick && state) {
    // LOG_DEBUG("STATE FANSENSE_2: %d\n", state);
    gpio_get_state(&bms_fan_sense2, &state);
  }
  set_battery_status_fan2(!state);
  // LOG_DEBUG("FAN1: %d\n", g_tx_struct.battery_status_fan1);
  // LOG_DEBUG("FAN2: %d\n", g_tx_struct.battery_status_fan2);
}

void bms_run_fan(void) {
  if (s_storage->current_storage.temperature >= BMS_FAN_TEMP_UPPER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_3, 100);
    LOG_DEBUG("GG MAX TEMP\n");
  } else if (s_storage->current_storage.temperature <= BMS_FAN_TEMP_LOWER_THRESHOLD) {
    pwm_set_dc(PWM_TIMER_3, 0);
  } else {
    pwm_set_dc(PWM_TIMER_3,
               (100 * (s_storage->current_storage.temperature - BMS_FAN_TEMP_LOWER_THRESHOLD)) /
                   (BMS_FAN_TEMP_UPPER_THRESHOLD - BMS_FAN_TEMP_LOWER_THRESHOLD));
  }
  prv_bms_fan_sense();
}

void bms_fan_init(BmsStorage *bms_storage) {
  s_storage = bms_storage;
  pwm_init(PWM_TIMER_1, BMS_FAN_PERIOD);
  pwm_set_pulse(PWM_TIMER_1, BMS_FAN_PERIOD);
  pwm_set_dc(PWM_TIMER_1, 50);

  gpio_init_pin(&bms_fan_sense1, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_init_pin(&bms_fan_sense2, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
}
